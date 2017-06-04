#include <iostream>
#include "Settings.h"
#include "Color.h"

using namespace std;

int main()
{
	cout << ANSI_BACKGROUND_BLUE << ANSI_WHITE << "FrendChat Server" << ANSI_RESET << endl;
	
	Settings settings;
	cout << "Listening on port " << settings.getPortNumber() << endl;
	return 0;
}