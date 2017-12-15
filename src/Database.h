#pragma once
#include <sqlite3.h>
#include <vector>
#include <string>
#include <mutex>

using namespace std;

static mutex dbMutex;

class Database
{
private:
  static Database *instance;

  const char *DBNAME = "frendchat.db";
  sqlite3 *db;

  Database();

  vector<vector<string>> sqlCommand(string);
  string toUpper(string);
  void printSQLResult(vector<vector<string>>); //Debug function

public:
  static Database *getInstance();
  void openDB();
  void closeDB();

  bool checkCredentials(string, string);
  bool checkUserExists(string);
  string getSalt(string);
  void registerUser(string, string, string, string);
};