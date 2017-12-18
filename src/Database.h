#pragma once
#include <sqlite3.h>
#include <vector>
#include <string>
#include <mutex>

using namespace std;

class Database
{
private:
  static Database *instance;
  mutex mu;

  const char *DBNAME = "frendchat.db";
  sqlite3 *db;

  Database();

  vector<vector<string>> sqlCommand(string);
  string toUpper(string);
  void printSQLResult(vector<vector<string>>); //Debug function

public:
  static Database *getInstance();
  void closeDB();

  bool checkCredentials(string, string);
  bool checkUserExists(string);
  string getSalt(string);
  string getColor(string);
  void registerUser(string, string, string, string);
  void updatePassword(string, string, string);
  void updateColor(string, string);
};