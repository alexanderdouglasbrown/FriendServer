#include <iostream>
#include <thread>
#include <string>
#include <signal.h>

#include "Socket.h"
#include "SocketIO.h"
#include "ClientSocket.h"
#include "Settings.h"
#include "Database.h"
#include "Broadcaster.h"
#include "Color.h"

using namespace std;

void quitSignal(int);
void clientSocketWorker(int);
void broadcastWorker();

int main()
{
	signal(SIGINT, quitSignal);
	cout << ANSI_BACKGROUND_BLUE << ANSI_WHITE << "Frend Chat Server" << ANSI_RESET << endl;

	Settings settings;
	Database *db = Database::getInstance();
	Socket socketObject;

	int serverHandle = socketObject.createSocket(settings.getPortNumber());

	cout << "Listening on port " << settings.getPortNumber() << "..." << endl;

	while (true)
	{
		int clientSocket = socketObject.acceptConnection(serverHandle);

		if (clientSocket != -1)
		{
			thread connection(clientSocketWorker, clientSocket);
			connection.detach();
		}
	}

	return 0;
}

void quitSignal(int signal)
{
	Database *db = Database::getInstance();
	db->closeDB();
	exit(EXIT_SUCCESS);
}

void clientSocketWorker(int clientSocket)
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

	Broadcaster *bc = Broadcaster::getInstance();
	bc->removeFromSocketList(clientSocket);
}