#pragma once

#include <string>

using namespace std;

class ClientSocket
{
  private:
    int clientSocket;

  public:
    ClientSocket(int);
    string readSocket();
    bool sendSocket(string);
};