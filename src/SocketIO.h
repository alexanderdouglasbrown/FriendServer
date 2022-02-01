#pragma once
#include <string>

using namespace std;

class SocketIO
{
private:
  int socket;
  string CRLF = "\r\n";

public:
  SocketIO();

  bool sendSocket(int, string);
  string readSocket(int);
  void closeSocket(int);
};