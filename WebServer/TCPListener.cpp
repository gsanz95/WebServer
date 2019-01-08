#include "TCPListener.h"

// Creates socket and starts listening
int TCPListener::Init()
{
	// Initializing WinSock
	WSADATA wsData;
	WORD ver = MAKEWORD(2,2);

	int winSock = WSAStartup(ver, &wsData);
	if(winSock != 0)
		return winSock;

	// Creating listening socket
	this->socket = ::socket(AF_INET, SOCK_STREAM, 0);

	if(this->socket == INVALID_SOCKET)
		return WSAGetLastError();

	// Fill sockaddr with ip addr and port
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(this->port);
	inet_pton(AF_INET, this->ipAddress, &hint.sin_addr);

	// Bind hint to socket
	if(bind(this->socket, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR)
		return WSAGetLastError();

	// Start listening on socket
	if(listen(this->socket, SOMAXCONN) == SOCKET_ERROR)
		return WSAGetLastError();

	// Zero master file desc
	FD_ZERO(&this->master);

	// Set listening socket to master in order to start listening
	// on other sockets
	FD_SET(this->socket, &this->master);

	return 0;
}

int TCPListener::Run()
{
	bool isRunning = true;

	// Get a copy for select()
	fd_set copy = this->master;

	// Count of communicating sockets
	int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

	for(int i=0; i<socketCount; ++i)
	{
		// Temp for socket being used
		int sock = copy.fd_array[i];

		// This is a listening socket
		if(sock == this->socket)
		{
			// Accept a new client
			int client = accept(sock, nullptr, nullptr);

			// Add client sock to master
			FD_SET(client, &this->master);

			// Client Connect Confirmation
			onClientConnected(client);
		}
		else // This is a client socket
		{
			// Byte storage
			char buff[MAX_BUFF_SIZE];

			// Clear buff
			memset(buff, 0, sizeof(buff));

			// Receive msg
			int bytesRecvd = recv(sock, buff, MAX_BUFF_SIZE, 0);
			if(bytesRecvd <= 0)
			{
				// Close client
				closesocket(sock);
				onClientDisconnected(sock);

				FD_CLR(sock, &this->master);
			}
			else
			{
				onMessageReceived(sock, buff, bytesRecvd);
			}
		}
	}

	// Remove listener socket and close it.
	FD_CLR(this->socket, &this->master);
	closesocket(this->socket);

	// Remove all client sockets and close them
	while(this->master.fd_count > 0)
	{
		int socketToClose = this->master.fd_array[0];

		FD_CLR(socketToClose, &this->master);
		closesocket(socketToClose);
	}

	// Clear WinSock
	WSACleanup();

	return 0;
}

void TCPListener::onClientConnected(int clientSocket)
{
}

void TCPListener::onClientDisconnected(int clientSocket)
{
}

void TCPListener::onMessageReceived(int clientSocket, const char* msg, int length)
{
}

void TCPListener::sendMessageToClient(int clientSocket, const char * msg, int length)
{
	send(clientSocket, msg, length, 0);
}

void TCPListener::broadcastToClients(int senderSocket, const char * msg, int length)
{
	for(int i=0; i<this->master.fd_count; ++i)
	{
		int sendSock = this->master.fd_array[i];

		if(sendSock != senderSocket && sendSock != this->socket)
			sendMessageToClient(sendSock, msg, length);
	}
}
