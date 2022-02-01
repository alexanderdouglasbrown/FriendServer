#include "SocketIO.h"

#include <string.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <netinet/in.h>

SocketIO::SocketIO()
{
}

string SocketIO::readSocket(int clientSocket)
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

bool SocketIO::sendSocket(int clientSocket, string message)
{
    auto crlfMessage = message + CRLF;

    int bufferLength = crlfMessage.length();
    int status;
    int offset = 0;

    while (offset < bufferLength)
    {
        char buffer[bufferLength] = {'\0'};
        strncpy(buffer, crlfMessage.c_str() + offset, sizeof(buffer));

        status = send(clientSocket, buffer + offset, bufferLength, 0);
        if (status == -1)
            return false;

        offset += status;
        bufferLength -= status;
    }

    return true;
}

void SocketIO::closeSocket(int clientSocket)
{
    close(clientSocket);
}