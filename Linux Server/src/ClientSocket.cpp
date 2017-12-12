#include "ClientSocket.h"
#include <string.h>
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
        strncpy(buffer, message.c_str() + offset, sizeof(buffer) - 1);

        status = send(clientSocket, buffer + offset, bufferLength, 0);
        if (status == -1)
            return false;

        offset += status;
        bufferLength -= status;
    }

    return true;
}