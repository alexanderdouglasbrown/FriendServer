#pragma once
#include <sqlite3.h>
#include <vector>
#include <string>

using namespace std;

class Database
{
private:
  const char *DBNAME = "frendchat.db";
  sqlite3 *db;

  vector<vector<string>> sqlCommand(string);
  void printSQLResult(vector<vector<string>>);

public:
  Database();
  void closeDB();

  bool checkCredentials(string, string);
};
