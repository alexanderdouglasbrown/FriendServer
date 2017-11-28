#include <iostream>
#include <thread>
#include <mutex>
#include <signal.h>

#include "Socket.h"
#include "Settings.h"
#include "Database.h"
#include "Color.h"

using namespace std;

void quitSignal(int);
int socketWorker(int, int);

int main()
{
	signal(SIGINT, quitSignal);
	cout << ANSI_BACKGROUND_BLUE << ANSI_WHITE << "Frend Chat Server" << ANSI_RESET << endl;

	Settings settings;
	Database *db = Database::getInstance();
	Socket socketObject;

	db->openDB();

	int serverHandle = socketObject.createSocket(settings.getPortNumber());

	cout << "Listening on port " << settings.getPortNumber() << "..." << endl;

	while (true)
	{
		int clientSocket = socketObject.acceptConnection(serverHandle);

		thread connection(socketWorker, clientSocket, serverHandle);
		connection.detach();
	}

	return 0;
}

void quitSignal(int signal)
{
	Database *db = Database::getInstance();
	db->closeDB();
	exit(EXIT_SUCCESS);
}

int socketWorker(int clientSocket, int serverHandle)
{
	cout << "Thread created." << endl;
	Socket socketObject;

	while (true)
	{
		string rcvMessage = socketObject.readSocket(clientSocket);
		if (rcvMessage == "DROP")
			break;
		else
			cout << rcvMessage;
	}
	cout << "Client disconnected." << endl;
}