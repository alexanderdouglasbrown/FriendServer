#pragma once
#include <netinet/in.h>
#include <string>

using namespace std;

class Socket
{
  private:
    const int BACKLOG = 5;
    int port, socketHandle;
    struct sockaddr_in socketAddress;

  public:
    Socket(int);
    int acceptConnection();
    string readSocket(int);
};