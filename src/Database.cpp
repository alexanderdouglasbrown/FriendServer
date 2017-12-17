#include "Database.h"
#include <iostream>
#include <vector>
#include <string>
#include <sqlite3.h>

using namespace std;

Database *Database::instance = nullptr;

Database::Database()
{
    if (sqlite3_open(DBNAME, &db) != SQLITE_OK)
    {
        cerr << "An error occured while trying to open the database." << endl;
        return;
    }

    string sql = "CREATE TABLE IF NOT EXISTS users("
                 "id INTEGER PRIMARY KEY,"
                 "username VARCHAR(50) NOT NULL,"
                 "hash CHAR(64) NOT NULL,"
                 "salt VARCHAR(100) NOT NULL,"
                 "color VARCHAR(7) NOT NULL);";

    vector<vector<string>> result = sqlCommand(sql);
}

Database *Database::getInstance()
{
    if (instance == nullptr)
    {
        instance = new Database();
    }

    return instance;
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
    lock_guard<mutex> lock(mu);

    sqlite3_stmt *statement;
    vector<vector<string>> result;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, 0) != SQLITE_OK)
    {
        cerr << "An error occurred during a database operation." << endl;
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

bool Database::checkCredentials(string username, string hash)
{
    username = toUpper(username);
    const char *usr = username.c_str();
    const char *hsh = hash.c_str();
    char *cleanSQL = sqlite3_mprintf("SELECT UPPER(username), hash FROM users "
                                     "WHERE UPPER(username) = UPPER('%q') AND hash = '%q'",
                                     usr, hsh);
    vector<vector<string>> result = sqlCommand(string(cleanSQL));
    sqlite3_free(cleanSQL);

    if (result.size() > 1)
        cerr << "Multiple entries returned when checking user credentials. This is no good." << endl;

    if (result.size() <= 0)
        return false;

    return (username == result[0][0] && hash == result[0][1]);
}

bool Database::checkUserExists(string username)
{
    username = toUpper(username);
    const char *usr = username.c_str();
    char *cleanSQL = sqlite3_mprintf("SELECT UPPER(username) FROM users "
                                     "WHERE UPPER(username) = UPPER('%q')",
                                     usr);
    vector<vector<string>> result = sqlCommand(string(cleanSQL));
    sqlite3_free(cleanSQL);

    if (result.size() > 1)
        cerr << "Multiple entries returned when checking user credentials. This is no good." << endl;

    if (result.size() <= 0)
        return false;

    return (username == result[0][0]);
}

string Database::getSalt(string username)
{
    username = toUpper(username);
    const char *usr = username.c_str();
    char *cleanSQL = sqlite3_mprintf("SELECT salt FROM users "
                                     "WHERE UPPER(username) = UPPER('%q')",
                                     usr);
    vector<vector<string>> result = sqlCommand(string(cleanSQL));
    sqlite3_free(cleanSQL);

    if (result.size() > 1)
        cerr << "Multiple entries returned when checking user credentials. This is no good." << endl;

    if (result.size() <= 0)
        return "";

    return result[0][0];
}

string Database::getColor(string username)
{
    username = toUpper(username);
    const char *usr = username.c_str();
    char *cleanSQL = sqlite3_mprintf("SELECT color FROM users "
                                     "WHERE UPPER(username) = UPPER('%q')",
                                     usr);
    vector<vector<string>> result = sqlCommand(string(cleanSQL));
    sqlite3_free(cleanSQL);

    if (result.size() > 1)
        cerr << "Multiple entries returned when checking user credentials. This is no good." << endl;

    if (result.size() <= 0)
        return "";

    return result[0][0];
}

void Database::registerUser(string username, string hash, string salt, string color)
{
    const char *usr = username.c_str();
    const char *hsh = hash.c_str();
    const char *slt = salt.c_str();
    const char *clr = color.c_str();

    char *cleanSQL = sqlite3_mprintf("INSERT INTO users(username, hash, salt, color) "
                                     "VALUES('%q','%q','%q','%q');",
                                     usr, hsh, slt, clr);
    vector<vector<string>> result = sqlCommand(string(cleanSQL));
    sqlite3_free(cleanSQL);

    return;
}

string Database::toUpper(string fixme)
{
    for (int i = 0; i < fixme.size(); i++)
    {
        if (fixme[i] >= 97 && fixme[i] <= 122)
            fixme[i] -= 32;
    }
    return fixme;
}