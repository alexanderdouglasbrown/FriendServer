#pragma once
#include <string>
#include <fstream>
using namespace std;

class Settings
{
private:
  int port = 41260;

  string settingsPath = "settings.conf";

  bool openFile(ifstream &);
  void parseFile(ifstream &);

  string toUpper(string);

public:
  Settings();
  int getPortNumber();
};
