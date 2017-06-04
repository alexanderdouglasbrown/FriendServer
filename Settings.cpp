#include "Settings.h"
#include <iostream>
#include <fstream>
#include <string>
#include "Color.h"
using namespace std;

Settings::Settings()
{
    ifstream settingsStream;
    if (openFile(settingsStream))
    {
        parseFile(settingsStream);
        settingsStream.close();
    }
    else
    {
        cerr << "Error: Failed to open settings.conf" << endl;
        cerr << "Using default port (41260)" << endl;
    }
}

int Settings::getPortNumber()
{
    return port;
}

bool Settings::openFile(ifstream &settingsStream)
{
    settingsStream.open(settingsPath);
    return settingsStream.is_open();
}

void Settings::parseFile(ifstream &settingsStream)
{
    cout << "Printing out settings.conf for now:\n\n";
    string line;
    int lineNum = 1;
    cout << ANSI_CYAN;
    while (getline(settingsStream, line))
    {
        //parseLine(line);
        cout << lineNum << ": " << line << endl;
        lineNum += 1;
    }
    cout << ANSI_RESET;
}

string Settings::toUpper(string fixme)
{
    for (int i = 0; i < fixme.size(); i++)
    {
        if (fixme[i] >= 97 && fixme[i] < 123)
            fixme[i] -= 32;
    }
    return fixme;
}