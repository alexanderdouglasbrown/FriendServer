#include "Socket.h"
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <netinet/in.h>

using namespace std;

Socket::Socket()
{
    memset(&socketAddress, 0, sizeof(socketAddress));
    socketAddress.sin6_family = AF_INET6;
    socketAddress.sin6_addr = in6addr_any;
}

int Socket::createSocket(int port)
{
    int socketHandle;

    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    int on = 1;
    socketAddress.sin6_port = htons(port);

    if ((socketHandle = socket(socketAddress.sin6_family, SOCK_STREAM, 0)) < 0)
    {
        cerr << "Failed to create socket." << endl;
        return socketHandle;
    }

    if (setsockopt(socketHandle, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0)
    {
        cerr << "Failed to set socket options." << endl;
        return socketHandle;
    }

    if (setsockopt(socketHandle, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        cerr << "Failed to set socket option (Send timeout)" << endl;
        return socketHandle;
    }

    if (bind(socketHandle, (struct sockaddr *)&socketAddress, sizeof(socketAddress)) < 0)
    {
        cerr << "Failed to bind socket." << endl;
        return socketHandle;
    }

    if (listen(socketHandle, BACKLOG) < 0)
    {
        cerr << "Failed to listen to socket." << endl;
        return socketHandle;
    }

    return socketHandle;
}

int Socket::acceptConnection(int socketHandle)
{
    int socketAddressLength = sizeof(socketAddress);
    int clientSocket;

    if ((clientSocket = accept(socketHandle, (struct sockaddr *)&socketAddress, (socklen_t *)&socketAddressLength)) < 0)
    {
        cerr << "An error occured while accepting a connection." << endl;
    }
    return clientSocket;
}