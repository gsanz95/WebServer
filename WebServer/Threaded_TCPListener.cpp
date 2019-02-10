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
	for(auto it=m_clients.begin(); it != m_clients.end(); ++it)
	{
		Client* c = *it;
		closesocket(c->socket);
	}

	m_clients.clear();
	//this->clients.clear();

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
	Threaded_TCPListener::removeDisconnectedClients();

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
			send(recipient->socket, recipient->messages.front().c_str(), strlen(recipient->messages.front().c_str()), 0);
			recipient->messages.pop();
		}
	}

	return;
}

void Threaded_TCPListener::broadcastToClients(Client *sender, const char * msg, int length)
{
	std::string text(msg);

	for(auto it=m_clients.begin(); it != m_clients.end(); ++it)
	{
		Client* c = *it;
		// Push into messages to be sent
		if (c != sender)
			c->messages.push(text);
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
		// Add client to queue
		Client* clientToAdd = new Client(client);
		m_clients.emplace(clientToAdd);

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

			// Disconnection Error
			std::cerr << err_buff;

			// Close client
			closesocket(sender->socket);

			// Add to removing queue
			m_clientsToRemove.push(sender);

			onClientDisconnected(sender);
		}
		else
		{
			onMessageReceived(sender, buff, bytesRecvd);
		}
	}

	return;
}

void Threaded_TCPListener::removeDisconnectedClients()
{
	// Remove clients until the queue is empty
	while(!m_clientsToRemove.empty())
	{
		m_clients.erase(m_clientsToRemove.front());
		m_clientsToRemove.pop();
	}
}
