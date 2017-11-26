#include <iostream>
#include <thread>
#include <mutex>

#include "Socket.h"
#include "Settings.h"
#include "Database.h"
#include "Color.h"

using namespace std;

int main()
{
	cout << ANSI_BACKGROUND_BLUE << ANSI_WHITE << "FrendChat Server" << ANSI_RESET << endl;

	Settings settings;
	Database db;
	Socket serverSocket(settings.getPortNumber());

	int clientSocket = serverSocket.acceptConnection();

	cout << serverSocket.readSocket(clientSocket) << endl;

	db.closeDB();
	return 0;
}
