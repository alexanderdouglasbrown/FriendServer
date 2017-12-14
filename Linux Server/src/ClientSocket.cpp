#include "ClientSocket.h"
#include "Database.h"
#include "../ext/sha256.h"

#include <string.h>
#include <mutex>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <netinet/in.h>

#include <iostream>

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
// Java Client is expecting CRLF in its repsonses end line

void ClientSocket::parseReply(string message)
{
    cout << "DEBUG: " << message;
    if (message == "FREND_SERVER\n")
    {
        //Client is looking for this exact string to verify it's connected to the right thing
        this->sendSocket("FREND_RECIEVED" + CRLF);
        return;
    }

    if (message.substr(0, 5) == "LOGIN")
    {
        //Strip \n
        int lineFeedPos;
        if ((lineFeedPos = message.find_last_of("\n")) != -1)
        {
            message.erase(lineFeedPos, 1);
        }

        //Split username and password
        string username, password;
        int passwordPos;
        passwordPos = message.find_first_of(" ");
        username = message.substr(5, passwordPos - 5);
        password = message.substr(passwordPos + 1);

        Database *db = Database::getInstance();

        //TODO: Hashing stuff

        dbMutex.lock();
        bool credentialsAccepted = db->checkCredentials(username, password);
        dbMutex.unlock();

        if (credentialsAccepted)
            this->sendSocket("CREDENTIALS_OKAY" + CRLF);
        else
            this->sendSocket("CREDENTIALS_DENIED" + CRLF);
    }
}