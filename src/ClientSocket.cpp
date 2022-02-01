#include "ClientSocket.h"
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
    return socketObject.readSocket(clientSocket);
}

bool ClientSocket::sendSocket(string message)
{
    return socketObject.sendSocket(clientSocket, message);
}

void ClientSocket::parseReply(string message)
{
    stripNewLine(message);
    if (message.substr(0, 11) == "FRIEND_CHAT")
        handleWelcome(message);

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
}

void ClientSocket::handleWelcome(string message)
{
    this->sendSocket("FRIEND_SERVER_WELCOME");
}

void ClientSocket::handleUpdateColor(string message)
{
    storedColor = message.substr(12);

    Database *db = Database::getInstance();
    db->updateColor(storedUsername, storedColor);

    Broadcaster *bc = Broadcaster::getInstance();
    bc->removeFromSocketList(clientSocket);
    bc->addToSocketList(clientSocket, storedUsername, storedColor);

    sendSocket("COLOR_UPDATED" + storedColor);
}

void ClientSocket::handleNewPass(string message)
{
    Database *db = Database::getInstance();
    string newPass = message.substr(8, message.length() - 8);

    long int time = static_cast<long int>(chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count());
    string salt = to_string(time);

    string hash = getHash(newPass, salt);

    db->updatePassword(storedUsername, hash, salt);

    sendSocket("PASS_UPDATED");
}

void ClientSocket::handleCheckPass(string message)
{
    Database *db = Database::getInstance();
    string salt = db->getSalt(storedUsername);
    string password = message.substr(8, message.length() - 8);
    string hash = getHash(password, salt);

    if (db->checkCredentials(storedUsername, hash))
        sendSocket("PASS_OK");
    else
        sendSocket("PASS_BAD");
}

void ClientSocket::handleLogin(string message)
{
    string username, password;

    int passwordPos = message.find_first_of(" ");
    username = message.substr(5, passwordPos - 5);
    password = message.substr(passwordPos + 1);

    Database *db = Database::getInstance();

    string salt = db->getSalt(username);

    string hash = getHash(password, salt);
    bool credentialsAccepted = db->checkCredentials(username, hash);

    if (!credentialsAccepted)
    {
        this->sendSocket("CREDENTIALS_DENIED");
        return;
    }

    this->sendSocket("CREDENTIALS_OKAY");
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
        this->sendSocket("USERNAME_IN_USE");
        return;
    }

    long int time = static_cast<long int>(chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count());
    salt = to_string(time);

    hash = getHash(password, salt);

    db->registerUser(username, hash, salt, color);

    storedUsername = username;
    storedColor = color;

    Broadcaster *bc = Broadcaster::getInstance();
    bc->addToSocketList(clientSocket, username, color);

    this->sendSocket("USER_REGISTERED");

    return;
}

void ClientSocket::handleBroadcastMessage(string message)
{
    if (storedUsername == "")
        return;

    string cleanMessage = message.substr(3, message.length() - 3);

    fixForHTML('&', "&amp;", cleanMessage);
    fixForHTML('<', "&lt;", cleanMessage);
    fixForHTML('>', "&gt;", cleanMessage);
    fixForHTML('\"', "&quot;", cleanMessage);
    fixForHTML('\'', "&#39;", cleanMessage);
    fixForHTML('\\', "&#92;", cleanMessage);

    string formattedMessage = storedColor + storedUsername + " " + cleanMessage;

    Broadcaster *bc = Broadcaster::getInstance();
    bc->broadcastMessage(formattedMessage);
}

string ClientSocket::getHash(string password, string salt)
{
    return sha256(password.substr(0, 256) + salt);
}

void ClientSocket::fixForHTML(char character, string replacement, string &toFix)
{
    string result = "";

    for (int i = 0; i < toFix.length(); i++)
    {
        if (toFix[i] == character)
            result += replacement;
        else
            result += toFix[i];
    }

    toFix.assign(result);
}

void ClientSocket::stripNewLine(string &message)
{
    int lineFeedPos;
    if ((lineFeedPos = message.find_last_of("\n")) != -1)
    {
        message.erase(lineFeedPos, 1);
    }
}