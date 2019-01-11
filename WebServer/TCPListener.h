#pragma once
#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#define MAX_BUFF_SIZE (4096)

#pragma comment(lib, "ws2_32.lib")

class TCPListener
{
public:

	TCPListener(const char* ipAddr, int port) : ipAddress(ipAddr), port(port) {}

	// Initialize the listening server
	virtual int Init();

	// Run the server
	virtual int Run();

protected:

	// Handler when client connects
	virtual void onClientConnected(int clientSocket);

	// Handler when client disconnects
	virtual void onClientDisconnected(int clientSocket);

	// Handler when message has been received from client
	virtual void onMessageReceived(int clientSocket, const char* msg, int length);

	// Message a specific client
	virtual void sendMessageToClient(int clientSocket, const char* msg, int length);

	// Broadcast message from one client to all others
	virtual void broadcastToClients(int sender, const char* msg, int length);

	const char* ipAddress;			// IP Address of the server
	int			port;				// Port Nr. for the web server
	int			socket;				// Listening socket

private:
	fd_set		master;				// Master file desc.
};