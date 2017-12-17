#include "ClientSocket.h"
#include "SocketIO.h"
#include "Database.h"
#include "Broadcaster.h"
#include "../ext/sha256.h"

#include <chrono>

using namespace std;

ClientSocket::ClientSocket(int clientSocket)
{
    this->clientSocket = clientSocket;
}

string ClientSocket::readSocket()
{
    SocketIO socketObject(clientSocket);
    return socketObject.readSocket();
}

bool ClientSocket::sendSocket(string message)
{
    SocketIO socketObject(clientSocket);
    return socketObject.sendSocket(message);
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

    if (message.substr(0, 3) == "TXT")
    {
        handleBroadcastMessage(message);
    }
    if (message == "LIST\n")
    {
        Broadcaster *bc = Broadcaster::getInstance();
        bc->requestUserList(clientSocket);
    }
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

    string salt = db->getSalt(username);

    string hash = sha256(password.substr(0, 256) + salt);
    bool credentialsAccepted = db->checkCredentials(username, hash);

    if (!credentialsAccepted)
    {
        this->sendSocket("CREDENTIALS_DENIED" + CRLF);
    }
    else
    {
        this->sendSocket("CREDENTIALS_OKAY" + CRLF);
        string color = db->getColor(username);

        storedUsername = username;
        storedColor = color;

        Broadcaster *bc = Broadcaster::getInstance();
        bc->addToSocketList(clientSocket, username, color);
    }

    return;
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

    bool usernameExists = db->checkUserExists(username);

    if (usernameExists)
    {
        this->sendSocket("USERNAME_IN_USE" + CRLF);
        return;
    }

    long int time = static_cast<long int>(chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count());
    salt = to_string(time);

    hash = sha256(password.substr(0, 256) + salt);

    db->registerUser(username, hash, salt, color);

    storedUsername = username;
    storedColor = color;

    Broadcaster *bc = Broadcaster::getInstance();
    bc->addToSocketList(clientSocket, username, color);

    this->sendSocket("USER_REGISTERED" + CRLF);

    return;
}

void ClientSocket::handleBroadcastMessage(string message)
{
    //Strip \n
    int lineFeedPos;
    if ((lineFeedPos = message.find_last_of("\n")) != -1)
    {
        message.erase(lineFeedPos, 1);
    }

    // Build formatted message
    string formattedMessage = "<div><b><font color='" + storedColor + "'>" + storedUsername + "</font>:</b> " + message.substr(3, message.length() - 3) + "</div>";

    Broadcaster *bc = Broadcaster::getInstance();
    bc->broadcastMessage(formattedMessage);
}