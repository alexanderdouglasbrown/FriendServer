#include "Broadcaster.h"
#include "SocketIO.h"

#include <vector>
#include <thread>
#include <iostream>

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

void Broadcaster::addToSocketList(int socket, string username, string color)
{
    clientInfo newClient;
    newClient.username = username;
    newClient.socket = socket;
    newClient.color = color;

    lock_guard<mutex> lock(mu);
    socketList.push_back(newClient);

    for (int i = 0; i < socketList.size() - 1; i++)
    {
        thread broadcaster(&Broadcaster::userJoinBroadcastWorker, this, socketList[i].socket, username, color);
        broadcaster.detach();
    }
}

void Broadcaster::userJoinBroadcastWorker(int socket, string username, string color)
{
    SocketIO socketObject(socket);
    socketObject.sendSocket("JOIN" + color + username + CRLF);
}

void Broadcaster::removeFromSocketList(int socket)
{
    lock_guard<mutex> lock(mu);
    string leavingUsername;

    for (int i = 0; i < socketList.size(); i++)
    {
        if (socketList[i].socket == socket)
        {
            leavingUsername = socketList[i].username;
            socketList.erase(socketList.begin() + i);
            break;
        }
    }

    for (int i = 0; i < socketList.size(); i++)
    {
        thread broadcaster(&Broadcaster::userLeaveBroadcastWorker, this, socketList[i].socket, leavingUsername);
        broadcaster.detach();
    }
}

void Broadcaster::userLeaveBroadcastWorker(int socket, string username)
{
    SocketIO socketObject(socket);
    socketObject.sendSocket("LEAVE" + username + CRLF);
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

void Broadcaster::messageBroadcastWorker(int socket, string message)
{
    SocketIO socketObject(socket);
    socketObject.sendSocket("BRD" + message + CRLF);
}

void Broadcaster::requestUserList(int socket)
{
    lock_guard<mutex> lock(mu);
    vector<clientInfo> usersOnline(socketList);
    mu.unlock();

    thread broadcaster(&Broadcaster::userListBroadcastWorker, this, socket, usersOnline);
    broadcaster.detach();
}

void Broadcaster::userListBroadcastWorker(int socket, vector<clientInfo> usersOnline)
{
    SocketIO socketObject(socket);
    for (int i = 0; i < usersOnline.size(); i++)
        socketObject.sendSocket("JOIN" + usersOnline[i].color + usersOnline[i].username + CRLF);
}

void Broadcaster::printBroadcastList()
{
    cout << "Socket\t\tColor\t\tUsername" << endl;
    for (int i = 0; i < socketList.size(); i++)
    {
        cout << socketList[i].socket << "\t\t" << socketList[i].color << "\t\t" << socketList[i].username << endl;
    }
    cout << "~~~~~~~~~~~~~~~~\n\n";
}