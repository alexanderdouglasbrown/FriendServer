#include "ClientSocket.h"
#include "Database.h"
#include "../ext/sha256.h"

#include <string.h>
#include <mutex>
#include <chrono>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <netinet/in.h>

using namespace std;

ClientSocket::ClientSocket(int clientSocket)
{
    this->clientSocket = clientSocket;
}

string ClientSocket::readSocket()
{
    const int BUFFER_SIZE = 2048;
    char buffer[BUFFER_SIZE] = {'\0'};
    int status;
    string message = "";

    while ((status = read(clientSocket, buffer, BUFFER_SIZE)) > 0)
    {
        message += string(buffer);

        if (buffer[BUFFER_SIZE - 1] == '\0')
            break;

        memset(buffer, '\0', BUFFER_SIZE);
    }

    if (status == 0)
        return "DROP";

    //Strip carriage return for predicatble messages between Windows/macOS/Linux
    int carriageReturnPos;
    if ((carriageReturnPos = message.find_last_of("\r")) != -1)
    {
        message.erase(carriageReturnPos, 1);
    }

    return message;
}

bool ClientSocket::sendSocket(string message)
{
    int bufferLength = message.length();
    int status;
    int offset = 0;

    while (offset < bufferLength)
    {
        char buffer[bufferLength] = {'\0'};
        strncpy(buffer, message.c_str() + offset, sizeof(buffer));

        status = send(clientSocket, buffer + offset, bufferLength, 0);
        if (status == -1)
            return false;

        offset += status;
        bufferLength -= status;
    }

    return true;
}

// Messages end with \n (\r is stripped)
// Java Client is expecting CRLF in its repsonses

void ClientSocket::parseReply(string message)
{
    if (message == "FREND_SERVER\n")
    {
        //Client is looking for this exact string to verify it's connected to the right thing
        this->sendSocket("FREND_RECIEVED" + CRLF);
        return;
    }

    if (message.substr(0, 5) == "LOGIN")
        handleLogin(message);

    if (message.substr(0, 3) == "REG")
        handleRegistration(message);
}

void ClientSocket::handleLogin(string message)
{
    //Strip \n
    int lineFeedPos;
    if ((lineFeedPos = message.find_last_of("\n")) != -1)
    {
        message.erase(lineFeedPos, 1);
    }

    //Split username and password
    string username, password;

    int passwordPos = message.find_first_of(" ");
    username = message.substr(5, passwordPos - 5);
    password = message.substr(passwordPos + 1);

    Database *db = Database::getInstance();

    dbMutex.lock();
    string salt = db->getSalt(username);
    string hash = sha256(password.substr(0,256) + salt);
    bool credentialsAccepted = db->checkCredentials(username, hash);
    dbMutex.unlock();

    if (credentialsAccepted)
        this->sendSocket("CREDENTIALS_OKAY" + CRLF);
    else
        this->sendSocket("CREDENTIALS_DENIED" + CRLF);
}

void ClientSocket::handleRegistration(string message)
{
    //Strip \n
    int lineFeedPos;
    if ((lineFeedPos = message.find_last_of("\n")) != -1)
    {
        message.erase(lineFeedPos, 1);
    }

    string username, password, color, salt, hash;
    int passwordPos = message.find_first_of(" ");
    color = message.substr(3, 7);
    username = message.substr(10, passwordPos - 10);
    password = message.substr(passwordPos + 1);

    Database *db = Database::getInstance();

    dbMutex.lock();
    bool usernameExists = db->checkUserExists(username);
    dbMutex.unlock();

    if (usernameExists)
    {
        this->sendSocket("USERNAME_IN_USE" + CRLF);
        return;
    }

    long int time = static_cast<long int>(chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count());
    salt = to_string(time);

    hash = sha256(password.substr(0, 256) + salt);

    dbMutex.lock();
    db->registerUser(username, hash, salt, color);
    dbMutex.unlock();

    this->sendSocket("USER_REGISTERED" + CRLF);

    return;
}