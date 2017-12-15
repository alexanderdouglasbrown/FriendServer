#pragma once

#include <string>

using namespace std;

class ClientSocket
{
  private:
    int clientSocket;
    string CRLF = "\r\n";

    void handleLogin(string);
    void handleRegistration(string);

  public:
    ClientSocket(int);
    string readSocket();
    bool sendSocket(string);

    void parseReply(string);
};