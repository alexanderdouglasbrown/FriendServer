#include "Broadcaster.h"

#include <vector>
#include <thread>

using namespace std;

Broadcaster *Broadcaster::instance = nullptr;

Broadcaster::Broadcaster()
{
}

Broadcaster *Broadcaster::getInstance()
{
    if (instance == nullptr)
    {
        instance = new Broadcaster();
    }

    return instance;
}

void Broadcaster::addToSocketList(int clientSocket, string username, string color)
{
    clientInfo newClient;
    newClient.username = username;
    newClient.socket = clientSocket;
    newClient.color = color;

    lock_guard<mutex> lock(mu);
    socketList.push_back(newClient);

    for (int i = 0; i < socketList.size() - 1; i++)
    {
        if (toUpper(socketList[i].username) == toUpper(username))
        {
            socketObject.closeSocket(socketList[i].socket);
            removeFromSocketList(socketList[i].socket, false);
            i--;
        }
        else
        {
            thread broadcaster(&Broadcaster::userJoinBroadcastWorker, this, socketList[i].socket, username, color);
            broadcaster.detach();
        }
    }
}

void Broadcaster::userJoinBroadcastWorker(int clientSocket, string username, string color)
{
    socketObject.sendSocket(clientSocket, "JOIN" + color + username + CRLF);
}

void Broadcaster::removeFromSocketList(int clientSocket)
{
    removeFromSocketList(clientSocket, true);
}

void Broadcaster::removeFromSocketList(int clientSocket, bool needsMutex)
{
    if (needsMutex)
        lock_guard<mutex> lock(mu);

    string leavingUsername = "";

    for (int i = 0; i < socketList.size(); i++)
    {
        if (socketList[i].socket == clientSocket)
        {
            leavingUsername = socketList[i].username;
            socketList.erase(socketList.begin() + i);
            break;
        }
    }

    if (leavingUsername == "")
        return;

    for (int i = 0; i < socketList.size(); i++)
    {
        thread broadcaster(&Broadcaster::userLeaveBroadcastWorker, this, socketList[i].socket, leavingUsername);
        broadcaster.detach();
    }
}

void Broadcaster::userLeaveBroadcastWorker(int clientSocket, string username)
{
    socketObject.sendSocket(clientSocket, "LEAVE" + username + CRLF);
}

void Broadcaster::broadcastMessage(string message)
{
    lock_guard<mutex> lock(mu);

    for (int i = 0; i < socketList.size(); i++)
    {
        thread broadcaster(&Broadcaster::messageBroadcastWorker, this, socketList[i].socket, message);
        broadcaster.detach();
    }
}

void Broadcaster::messageBroadcastWorker(int clientSocket, string message)
{
    socketObject.sendSocket(clientSocket, "BRD" + message + CRLF);
}

void Broadcaster::requestUserList(int clientSocket)
{
    lock_guard<mutex> lock(mu);
    vector<clientInfo> usersOnline(socketList);
    mu.unlock();

    thread broadcaster(&Broadcaster::userListBroadcastWorker, this, clientSocket, usersOnline);
    broadcaster.detach();
}

void Broadcaster::userListBroadcastWorker(int clientSocket, vector<clientInfo> usersOnline)
{
    for (int i = 0; i < usersOnline.size(); i++)
        socketObject.sendSocket(clientSocket, "JOIN" + usersOnline[i].color + usersOnline[i].username + CRLF);
}

string Broadcaster::toUpper(string fixme)
{
    for (int i = 0; i < fixme.size(); i++)
    {
        if (fixme[i] >= 97 && fixme[i] <= 122)
            fixme[i] -= 32;
    }
    return fixme;
}