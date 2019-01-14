#pragma once
#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <thread>
#include <vector>

#include "TCPListener.h"
#include "BlockingQueue.h"

#pragma comment(lib, "ws2_32.lib")

#ifndef THREADED_TCP_LISTENER_H
#define THREADED_TCP_LISTENER_H

typedef struct Client
{
	Client(int sock);
	~Client();

	int socket;
	//BlockingQueue<const char*> messages;
	BlockingQueue<std::string> messages;
} Client;

class Threaded_TCPListener : TCPListener
{
public:

	Threaded_TCPListener(const char* ip_address, int port)
		: TCPListener(ip_address, port) {}

	int Init();

	int Run();

	~Threaded_TCPListener();

protected:

	void onClientConnected(Client *client);

	void onClientDisconnected(Client *client);

	void onMessageReceived(Client *sender, const char * msg, int length);

	void broadcastToClients(Client *sender, const char * msg, int length);

	void sendToClient(Client *recipient);

	void receiveFromClient(Client *sender);

private:

	void acceptClient();

	std::vector<Client*> clients;
};
#endif