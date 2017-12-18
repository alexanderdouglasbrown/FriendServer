#pragma once
#include <string>
#include "SocketIO.h"

using namespace std;

class ClientSocket
{
private:
  int clientSocket;
  SocketIO socketObject;
  string storedUsername = "";
  string storedColor = "";
  string CRLF = "\r\n";

  void handleLogin(string);
  void handleRegistration(string);
  void handleBroadcastMessage(string);
  void handleCheckPass(string);
  void handleNewPass(string);
  void handleUpdateColor(string);

  void stripNewLine(string &);

public:
  ClientSocket(int);
  string readSocket();
  bool sendSocket(string);

  void parseReply(string);
};