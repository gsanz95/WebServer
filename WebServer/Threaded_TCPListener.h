#pragma once
#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <thread>
#include "TCPListener.h"
#include <unordered_set>
#include <future>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

class Threaded_TCPListener : TCPListener
{
public:

	Threaded_TCPListener(const char* ip_address, int port)
		: TCPListener(ip_address, port) {}

	int Init();

	int Run();

	~Threaded_TCPListener();

protected:

	void onClientConnected(int clientSocket);

	void onClientDisconnected(int clientSocket);

	void onMessageReceived(int clientSocket, const char * msg, int length);

	void sendMessageToClient(int clientSocket, const char* msg, int length);

	void broadcastToClients(int senderSocket, const char * msg, int length);

private:

	void receiveFromSocket(int receivingSocket);

	void listenForClients();

	std::future<void> listeningFuture;
	std::unordered_set<int> clients;
};
