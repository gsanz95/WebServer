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
		closesocket(this->clients[i]->socket);
	}

	this->clients.clear();

	// Clear WinSock
	WSACleanup();
}

void Threaded_TCPListener::onClientConnected(Client *client)
{
	std::cout << "New Client Connected!\n";
}

void Threaded_TCPListener::onClientDisconnected(Client *client)
{
	std::cout << "Client Disconnected!\n";
}

void Threaded_TCPListener::onMessageReceived(Client *client, const char* msg, int length)
{
	Threaded_TCPListener::broadcastToClients(client, msg, length);

	return;
}

void Threaded_TCPListener::sendToClient(Client *recipient)
{
	bool isAlive = true;

	// Send message when available
	while(isAlive)
	{
		if(recipient->messages.size() > 0)
		{
			//send(recipient->socket, recipient->messages.front(), strlen(recipient->messages.front()), 0);
			send(recipient->socket, recipient->messages.front().c_str(), strlen(recipient->messages.front().c_str()), 0);
			std::cout << "Sent: " << recipient->messages.front() << std::endl;
			recipient->messages.pop();
		}
	}

	return;
}

void Threaded_TCPListener::broadcastToClients(Client *sender, const char * msg, int length)
{
	std::string text(msg);
	std::cout << "Trying to send:" << text << std::endl;
	// Iterate over all clients
	for (int i=0; i<this->clients.size(); ++i)
	{
		// Push into messages to be sent
		if(this->clients[i] != sender)
			this->clients[i]->messages.push(text);
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
		Client* clientToAdd = new Client(client);
		this->clients.emplace_back(clientToAdd);

		// Client Connect Confirmation
		onClientConnected(clientToAdd);

		// Create a threads for client
		std::thread receive(&Threaded_TCPListener::receiveFromClient, this, clientToAdd);
		std::thread send(&Threaded_TCPListener::sendToClient, this, clientToAdd);

		// Set to be independent
		receive.detach();
		send.detach();
	}

	return;
}

void Threaded_TCPListener::receiveFromClient(Client *sender)
{
	bool isAlive = true;

	while(isAlive)
	{
		// Byte storage
		char buff[MAX_BUFF_SIZE];

		// Clear buff
		memset(buff, 0, sizeof(buff));

		// Receive msg
		int bytesRecvd = recv(sender->socket, buff, MAX_BUFF_SIZE, 0);
		if(bytesRecvd <= 0)
		{
			isAlive = false;
			char err_buff[1024];
			strerror_s(err_buff, bytesRecvd);

			std::cerr << err_buff;
			// Close client
			// TO-DO: Erase client class from vector
			closesocket(sender->socket);

			onClientDisconnected(sender);
		}
		else
		{
			onMessageReceived(sender, buff, bytesRecvd);
		}
	}

	return;
}

Client::Client(int sock)
{
	this->socket = sock;
	//this->messages = BlockingQueue<const char*>();
	this->messages = BlockingQueue<std::string>();
}
