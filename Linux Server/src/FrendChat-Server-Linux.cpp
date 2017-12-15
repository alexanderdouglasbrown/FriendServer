#include <iostream>
#include <thread>
#include <string>
#include <signal.h>

#include "Socket.h"
#include "ClientSocket.h"
#include "Settings.h"
#include "Database.h"
#include "Broadcaster.h"
#include "Color.h"

using namespace std;

void quitSignal(int);
int clientSocketWorker(int, int);

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

		thread connection(clientSocketWorker, clientSocket, serverHandle);
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

int clientSocketWorker(int clientSocket, int serverHandle)
{
	ClientSocket socketObject(clientSocket);

	while (true)
	{
		string rcvMessage = socketObject.readSocket();
		if (rcvMessage == "DROP")
			break;
		else
			socketObject.parseReply(rcvMessage);
	}
}