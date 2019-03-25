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

// Messages end with \n (\r is stripped)
// Java Client is expecting CRLF in its repsonses

void ClientSocket::parseReply(string message)
{
    stripNewLine(message);
    if (message.substr(0, 10) == "FREND_CHAT")
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
    int version, subversion; //Client version
    string versionString = message.substr(11, message.length());

    try
    {
        version = stoi(versionString.substr(versionString.find_first_of("_") + 1, versionString.find_last_of("_") - versionString.find_first_of("_") - 1));
        subversion = stoi(versionString.substr(versionString.find_last_of("_") + 1, versionString.length() - versionString.find_last_of("_") - 1));
    }
    catch (exception)
    {
        version = 0;
        subversion = 0;
    }

    if (version == 1 && subversion == 1) //Still compatible client
        this->sendSocket("FREND_SERVER_VER_1_01" + CRLF);
    else if (version == 1 && subversion == 2)
        this->sendSocket("FREND_SERVER_WELCOME" + CRLF);
    else
        this->sendSocket("FREND_SERVER_UNSUPPORTED_CLIENT" + CRLF);
}

void ClientSocket::handleUpdateColor(string message)
{
    storedColor = message.substr(12);

    Database *db = Database::getInstance();
    db->updateColor(storedUsername, storedColor);

    Broadcaster *bc = Broadcaster::getInstance();
    bc->removeFromSocketList(clientSocket);
    bc->addToSocketList(clientSocket, storedUsername, storedColor);

    sendSocket("COLOR_UPDATED" + storedColor + CRLF);
}

void ClientSocket::handleNewPass(string message)
{
    Database *db = Database::getInstance();
    string newPass = message.substr(8, message.length() - 8);

    long int time = static_cast<long int>(chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count());
    string salt = to_string(time);

    string hash = getHash(newPass, salt);

    db->updatePassword(storedUsername, hash, salt);

    sendSocket("PASS_UPDATED" + CRLF);
}

void ClientSocket::handleCheckPass(string message)
{
    Database *db = Database::getInstance();
    string salt = db->getSalt(storedUsername);
    string password = message.substr(8, message.length() - 8);
    string hash = getHash(password, salt);

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

    string hash = getHash(password, salt);
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

    hash = getHash(password, salt);

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
    //Would cause a problem if the replacement has the char in it after the first char
    //But for what I need, this will do
    for (int i = 0; i < toFix.length(); i++)
    {
        if (toFix[i] == character)
        {
            toFix.erase(i, 1);
            toFix.insert(i, replacement);
        }
    }
}

void ClientSocket::stripNewLine(string &message)
{
    int lineFeedPos;
    if ((lineFeedPos = message.find_last_of("\n")) != -1)
    {
        message.erase(lineFeedPos, 1);
    }
}