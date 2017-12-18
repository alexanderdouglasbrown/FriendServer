#pragma once
#include <string>

using namespace std;

class SocketIO
{
private:
  int socket;

public:
  SocketIO();

  bool sendSocket(int, string);
  string readSocket(int);
  void closeSocket(int);
};