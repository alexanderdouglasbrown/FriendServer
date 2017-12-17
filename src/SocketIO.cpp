#include "SocketIO.h"

#include <string.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <netinet/in.h>

SocketIO::SocketIO(int socket)
{
    this->socket = socket;
}

string SocketIO::readSocket()
{
    const int BUFFER_SIZE = 2048;
    char buffer[BUFFER_SIZE] = {'\0'};
    int status;
    string message = "";

    while ((status = read(socket, buffer, BUFFER_SIZE)) > 0)
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

bool SocketIO::sendSocket(string message)
{
    int bufferLength = message.length();
    int status;
    int offset = 0;

    while (offset < bufferLength)
    {
        char buffer[bufferLength] = {'\0'};
        strncpy(buffer, message.c_str() + offset, sizeof(buffer));

        status = send(socket, buffer + offset, bufferLength, 0);
        if (status == -1)
            return false;

        offset += status;
        bufferLength -= status;
    }

    return true;
}