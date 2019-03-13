#include <iostream>
#include <thread>
#include <string>
#include <signal.h>
#include <memory>

#include "Socket.h"
#include "SocketIO.h"
#include "ClientSocket.h"
#include "Database.h"
#include "Broadcaster.h"
#include "Color.h"

#define DEFAULT_PORT_NUMBER 41260

using namespace std;

void quitSignal(int);
void clientSocketWorker(int);
void broadcastWorker();

int main(int argc, char *argv[])
{
	int portNumber = DEFAULT_PORT_NUMBER;

	if (argc >= 2)
		portNumber = atoi(argv[1]);

	if (portNumber == 0)
		portNumber = DEFAULT_PORT_NUMBER;


	signal(SIGINT, quitSignal);
	cout << ANSI_BACKGROUND_BLUE << ANSI_WHITE << "Frend Chat Server" << ANSI_RESET << endl;

	Database *db = Database::getInstance();
	Socket socketObject;

	int serverHandle = socketObject.createSocket(portNumber);

	cout << "Listening on port " << portNumber << "..." << endl;

	while (true)
	{
		int clientSocket = socketObject.acceptConnection(serverHandle);

		if (clientSocket != -1)
		{
			thread connection(clientSocketWorker, clientSocket);
			connection.detach();
		} else {
			break;
		}
	}

	return 0;
}

void quitSignal(int signal)
{
	Database *db = Database::getInstance();
	Broadcaster *broadcaster = Broadcaster::getInstance();
	db->closeDB();

	delete db;
	delete broadcaster;
	
	exit(EXIT_SUCCESS);
}

void clientSocketWorker(int clientSocket)
{
	unique_ptr<ClientSocket> socketObject(new ClientSocket(clientSocket));

	while (true)
	{
		string rcvMessage = socketObject->readSocket();
		if (rcvMessage == "DROP")
			break;
		else
			socketObject->parseReply(rcvMessage);
	}

	Broadcaster *bc = Broadcaster::getInstance();
	bc->removeFromSocketList(clientSocket);
}