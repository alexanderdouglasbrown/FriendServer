#pragma once
#include <string>

using namespace std;

class SocketIO
{
  private:
    int socket;

  public:
    SocketIO(int);
    bool sendSocket(string);
    string readSocket();
};