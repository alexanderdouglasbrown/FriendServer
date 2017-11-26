#include "Socket.h"
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <netinet/in.h>

using namespace std;

Socket::Socket(int port)
{
    this->port = port;

    socketAddress.sin_family = AF_INET;
    socketAddress.sin_addr.s_addr = INADDR_ANY;
    socketAddress.sin_port = htons(port);

    if ((socketHandle = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        cerr << "Failed to create socket." << endl;
        return;
    }

    if (bind(socketHandle, (struct sockaddr *)&socketAddress, sizeof(socketAddress)) < 0)
    {
        cerr << "Failed to bind socket." << endl;
        return;
    }

    if (listen(socketHandle, BACKLOG) < 0)
    {
        cerr << "Failed to listen to socket." << endl;
        return;
    }

    cout << "Listening on port " << port << "..." << endl;
}

int Socket::acceptConnection()
{
    int socketAddressLength = sizeof(socketAddress);
    int clientSocket;

    if ((clientSocket = accept(socketHandle, (struct sockaddr *)&socketAddress, (socklen_t *)&socketAddressLength)) < 0)
    {
        cerr << "An error occured while accepting a connection." << endl;
    }
    return clientSocket;
}

string Socket::readSocket(int clientSocket)
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

    return message;
}