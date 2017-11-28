#include "Database.h"
#include <iostream>
#include <vector>
#include <string>
#include <sqlite3.h>

using namespace std;

Database *Database::instance = nullptr;

Database::Database()
{
}

Database *Database::getInstance()
{
    if (instance == nullptr)
    {
        instance = new Database();
    }

    return instance;
}

void Database::openDB()
{
    if (sqlite3_open(DBNAME, &db) != SQLITE_OK)
    {
        cerr << "An error occured while trying to open the database." << endl;
        return;
    }

    string sql = "CREATE TABLE IF NOT EXISTS users("
                 "id INTEGER PRIMARY KEY,"
                 "username VARCHAR(255) NOT NULL,"
                 "password VARCHAR(255) NOT NULL);";

    vector<vector<string>> result = sqlCommand(sql);
}

//For debug since I'm doing everything without an IDE
void Database::printSQLResult(vector<vector<string>> result)
{
    if (result.size() == 0)
    {
        cout << "No result" << endl;
        return;
    }
    for (int i = 0; i < result.size(); i++)
    {
        for (int k = 0; k < result[i].size(); k++)
        {
            cout << result[i][k];
            if (k < result[i].size() - 1)
            {
                cout << " | ";
            }
        }
        cout << endl;
    }
}

vector<vector<string>> Database::sqlCommand(string sql)
{
    sqlite3_stmt *statement;
    vector<vector<string>> result;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, 0) != SQLITE_OK)
    {
        cerr << "\nAn error occurred during a database operation." << endl;
    }
    else
    {
        int numColumns = sqlite3_column_count(statement);

        while (sqlite3_step(statement) == SQLITE_ROW)
        {
            vector<string> row;

            for (int i = 0; i < numColumns; i++)
            {
                row.push_back(string((char *)sqlite3_column_text(statement, i)));
            }

            result.push_back(row);
        }
        sqlite3_finalize(statement);
    }

    return result;
}

void Database::closeDB()
{
    if (db != nullptr)
        sqlite3_close(db);
}

bool Database::checkCredentials(string username, string password)
{
    const char *usr = username.c_str();
    const char *pswd = password.c_str();
    char *cleanSQL = sqlite3_mprintf("SELECT username, password FROM users "
                                     "WHERE username = '%q' AND password = '%q'",
                                     usr, pswd);
    vector<vector<string>> result = sqlCommand(string(cleanSQL));
    sqlite3_free(cleanSQL);

    if (result.size() > 1)
        cerr << "Multiple entries returned when checking user credentials. This is no good." << endl;

    if (result.size() <= 0)
        return false;

    return (username == result[0][0] && password == result[0][1]);
}