#pragma once
#include "ClientSocket.h"

#include <mutex>
#include <vector>
#include <string>

using namespace std;

class Broadcaster
{
private:
  struct clientInfo
  {
    int socket;
    string username;
    string color;
  };

  string CRLF = "\r\n";
  mutex mu;
  vector<clientInfo> socketList;

  static Broadcaster *instance;
  Broadcaster();
  void printBroadcastList(); //Debug function.
  void messageBroadcastWorker(int, string);
  void userJoinBroadcastWorker(int, string, string);
  void userLeaveBroadcastWorker(int, string);
  void userListBroadcastWorker(int, vector<clientInfo>);

public:
  static Broadcaster *getInstance();
  void addToSocketList(int, string, string);
  void removeFromSocketList(int);
  void broadcastMessage(string);
  void requestUserList(int);

};