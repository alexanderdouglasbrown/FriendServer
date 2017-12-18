#include "ClientSocket.h"
#include "Database.h"
#include "Broadcaster.h"
#include "../ext/sha256.h"

#include <chrono>
// #include <iostream>

using namespace std;

ClientSocket::ClientSocket(int clientSocket)
{
    this->clientSocket = clientSocket;
}

string ClientSocket::readSocket()
{
    return socketObject.readSocket(clientSocket);
}

bool ClientSocket::sendSocket(string message)
{
    return socketObject.sendSocket(clientSocket, message);
}

// Messages end with \n (\r is stripped)
// Java Client is expecting CRLF in its repsonses

void ClientSocket::parseReply(string message)
{
    stripNewLine(message);
    if (message.substr(0, 10) == "FREND_CHAT")

        this->sendSocket("FREND_SERVER_VER_1_00" + CRLF);

    else if (message.substr(0, 5) == "LOGIN")
        handleLogin(message);

    else if (message.substr(0, 3) == "REG")
        handleRegistration(message);

    else if (message.substr(0, 3) == "TXT")
        handleBroadcastMessage(message);

    else if (message == "LIST")
    {
        Broadcaster *bc = Broadcaster::getInstance();
        bc->requestUserList(clientSocket);
    }

    else if (message.substr(0, 8) == "CHK_PASS")
        handleCheckPass(message);

    else if (message.substr(0, 8) == "NEW_PASS")
        handleNewPass(message);

    else if (message.substr(0, 12) == "UPDATE_COLOR")
        handleUpdateColor(message);

    // else
    //     cerr << "Unrecognized command: " << message << endl;
}

void ClientSocket::handleUpdateColor(string message)
{
    storedColor = message.substr(12);

    Database *db = Database::getInstance();
    db->updateColor(storedUsername, storedColor);

    Broadcaster *bc = Broadcaster::getInstance();
    bc->removeFromSocketList(clientSocket);
    bc->addToSocketList(clientSocket, storedUsername, storedColor);

    sendSocket("COLOR_UPDATED" + CRLF);
}

void ClientSocket::handleNewPass(string message)
{
    Database *db = Database::getInstance();
    string newPass = message.substr(8, message.length() - 8);

    long int time = static_cast<long int>(chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count());
    string salt = to_string(time);

    string hash = sha256(newPass.substr(0, 256) + salt);

    db->updatePassword(storedUsername, hash, salt);

    sendSocket("PASS_UPDATED" + CRLF);
}

void ClientSocket::handleCheckPass(string message)
{
    Database *db = Database::getInstance();
    string salt = db->getSalt(storedUsername);
    string password = message.substr(8, message.length() - 8);
    string hash = sha256(password.substr(0, 256) + salt);

    if (db->checkCredentials(storedUsername, hash))
        sendSocket("PASS_OK" + CRLF);
    else
        sendSocket("PASS_BAD" + CRLF);
}

void ClientSocket::handleLogin(string message)
{
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
        return;
    }

    this->sendSocket("CREDENTIALS_OKAY" + CRLF);
    string color = db->getColor(username);

    storedUsername = username;
    storedColor = color;

    Broadcaster *bc = Broadcaster::getInstance();
    bc->addToSocketList(clientSocket, username, color);

    return;
}

void ClientSocket::handleRegistration(string message)
{
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
    // Build formatted message
    string formattedMessage = "<div><b><font color='" + storedColor + "'>" + storedUsername + "</font>:</b> " + message.substr(3, message.length() - 3) + "</div>";

    Broadcaster *bc = Broadcaster::getInstance();
    bc->broadcastMessage(formattedMessage);
}

void ClientSocket::stripNewLine(string &message)
{
    int lineFeedPos;
    if ((lineFeedPos = message.find_last_of("\n")) != -1)
    {
        message.erase(lineFeedPos, 1);
    }
}