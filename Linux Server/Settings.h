#pragma once
#include <string>
#include <fstream>
using namespace std;

class Settings
{
private:
  const int DEFAULTPORT = 41260;
  const string SETTINGSPATH = "settings.conf";
  
  int port = DEFAULTPORT;

  bool openFile(ifstream &);
  void parseFile(ifstream &);
  void parseCommand(string, string);

  string toUpper(string);

public:
  Settings();
  int getPortNumber();
};