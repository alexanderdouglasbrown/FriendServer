#pragma once

#include <string>

using namespace std;

class ClientSocket
{
  private:
    int clientSocket;
    string CRLF = "\r\n";

  public:
    ClientSocket(int);
    string readSocket();
    bool sendSocket(string);

    void parseReply(string);
};