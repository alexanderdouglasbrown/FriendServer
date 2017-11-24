#include <iostream>
//#include <signal.h> //TODO run code when user exits to close DB and stuff
#include "Settings.h"
#include "Database.h"
#include "Color.h"

using namespace std;

int main()
{
	cout << ANSI_BACKGROUND_BLUE << ANSI_WHITE << "FrendChat Server" << ANSI_RESET << endl;
	
	Settings settings;
	Database db;
	
	cout << "Listening on port " << settings.getPortNumber() << endl;
	cout << "Check for Alex: " << db.checkCredentials("Alex", "password") << endl;
	cout << "Check for Lisa: " << db.checkCredentials("Lisa", "shouldfail") << endl;

	db.closeDB();
	return 0;
}