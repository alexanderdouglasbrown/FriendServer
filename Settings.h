#pragma once
#include <string>
#include <fstream>
using namespace std;

class Settings
{
private:
  const int DEFAULTPORT = 41260;
  int port = DEFAULTPORT;

  string settingsPath = "settings.conf";

  bool openFile(ifstream &);
  void parseFile(ifstream &);
  void parseCommand(string, string);

  string toUpper(string);

public:
  Settings();
  int getPortNumber();
};