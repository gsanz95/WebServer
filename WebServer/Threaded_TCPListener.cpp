#include "Threaded_TCPListener.h"

int Threaded_TCPListener::Init()
{
	// Initializing WinSock
	WSADATA wsData;
	WORD ver = MAKEWORD(2,2);

	int winSock = WSAStartup(ver, &wsData);
	if(winSock != 0)
		return winSock;

	// Creating listening socket
	this->socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

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

	return 0;
}

int Threaded_TCPListener::Run()
{
	bool isRunning = true;

	// Accept client
	this->acceptClient();

	return 0;
}

Threaded_TCPListener::~Threaded_TCPListener()
{
	// Remove listener socket and close it.
	closesocket(this->socket);

	// Close all client sockets
	for(int i=0; i<this->clients.size(); ++i)
	{
		closesocket(this->clients[i].socket);
	}

	this->clients.clear();

	// Clear WinSock
	WSACleanup();
}

void Threaded_TCPListener::onClientConnected(int clientSocket)
{
	std::cout << "New Client Connected!\n";
}

void Threaded_TCPListener::onClientDisconnected(int clientSocket)
{
	std::cout << "Client Disconnected!\n";
}

void Threaded_TCPListener::onMessageReceived(int clientSocket, const char* msg, int length)
{
	Threaded_TCPListener::broadcastToClients(clientSocket, msg, length);

	return;
}

void Threaded_TCPListener::sendMessageToClient(Client& recipient)
{
	bool isAlive = true;

	// Send message when available
	while(isAlive)
	{
		if(recipient.messages.size() > 0)
			send(recipient.socket, recipient.messages.front(), strlen(recipient.messages.front()) + 1, 0);
	}

	return;
}

void Threaded_TCPListener::broadcastToClients(int senderSocket, const char * msg, int length)
{
	// Iterate over all clients
	for (int i=0; i<this->clients.size(); ++i)
	{
		// Push into messages to be sent
		if(this->clients[i].socket != senderSocket)
			this->clients[i].messages.push(msg);
	}

	return;
}

void Threaded_TCPListener::acceptClient()
{
	int client = accept(this->socket, nullptr, nullptr);

	// Error
	if(client == INVALID_SOCKET)
	{
		std::printf("Accept Err: %d\n", WSAGetLastError());
	}
	// Add client to clients queue
	else
	{
		/*
		//Enabling non-blocking
		u_long iMode = 1;
		int result = ioctlsocket(client, FIONBIO, &iMode);
		if(result != NO_ERROR)
			std::cerr << "ioctlsocket exit with err: " << result << std::endl;
		else
		{}
		*/
		// Add client to queue
		Client clientToAdd(client);
		this->clients.emplace_back(clientToAdd);

		// Client Connect Confirmation
		onClientConnected(client);

		// Create a threads for client
		std::thread recvThread(&Threaded_TCPListener::receiveFromClient, this, clientToAdd);
		std::thread sendThread(&Threaded_TCPListener::sendMessageToClient, this, clientToAdd);

		clientToAdd.recvThread = std::ref(recvThread);
		clientToAdd.sendThread = std::ref(sendThread);

		// Detach both threads
		recvThread.detach();
	}

	return;
}

void Threaded_TCPListener::receiveFromClient(Client& client)
{
	bool isAlive = true;

	while(true)
	{
		// Byte storage
		char buff[MAX_BUFF_SIZE];

		// Clear buff
		memset(buff, 0, sizeof(buff));

		// Receive msg
		int bytesRecvd = recv(client.socket, buff, MAX_BUFF_SIZE, 0);
		if(bytesRecvd <= 0)
		{
			isAlive = false;
			char err_buff[1024];
			strerror_s(err_buff, bytesRecvd);

			std::cerr << err_buff;
			// Close client
			// TO-DO: Erase client class from vector
			closesocket(client.socket);

			onClientDisconnected(client.socket);
		}
		else
		{
			onMessageReceived(client.socket, buff, bytesRecvd);
		}
	}

	return;
}
