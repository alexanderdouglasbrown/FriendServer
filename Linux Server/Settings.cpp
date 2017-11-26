#include "Settings.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

Settings::Settings()
{
    ifstream settingsStream;
    settingsStream.open(SETTINGS_PATH);

    if (settingsStream.is_open())
    {
        parseFile(settingsStream);
        settingsStream.close();
    }
    else
    {
        cerr << "Error: Failed to open " << SETTINGS_PATH << "." << endl;
    }
}

int Settings::getPortNumber()
{
    return port;
}

void Settings::parseFile(ifstream &settingsStream)
{
    string line, command, value;

    //Read line by line from settings.conf
    while (getline(settingsStream, line))
    {
        istringstream iss(line);
        iss >> command;

        //Send to parse if not a comment
        if (command.length() >= 1)
        {
            if (command.at(0) != '#')
            {
                iss >> value;
                parseCommand(command, value);
            }
        }
    }
}

void Settings::parseCommand(string command, string value)
{
    command = toUpper(command);

    if (command == "PORT")
    {
        try
        {
            port = stoi(value);
        }
        catch (exception &e)
        {
            cerr << "Error setting port. Using default port. Please check settings.conf." << endl;
        }
    }
}

string Settings::toUpper(string fixme)
{
    for (int i = 0; i < fixme.size(); i++)
    {
        if (fixme[i] >= 97 && fixme[i] <= 122)
            fixme[i] -= 32;
    }
    return fixme;
}