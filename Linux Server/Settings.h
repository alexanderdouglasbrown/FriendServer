#pragma once
#include <string>
#include <fstream>
using namespace std;

class Settings
{
private:
  const int DEFAULT_PORT = 41260;
  const string SETTINGS_PATH = "settings.conf";
  
  int port = DEFAULT_PORT;

  bool openFile(ifstream &);
  void parseFile(ifstream &);
  void parseCommand(string, string);

  string toUpper(string);

public:
  Settings();
  int getPortNumber();
};