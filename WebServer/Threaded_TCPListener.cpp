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

	// Start Listening on new thread
	using namespace  std::chrono_literals;

	std::packaged_task<void()> listener(std::thread(&Threaded_TCPListener::listenForClients);

	this->listeningFuture = listener.get_future();

	std::thread t(std::move(listener),this);

	return 0;
}

int Threaded_TCPListener::Run()
{
	bool isRunning = true;

	{
		using namespace std::chrono_literals;

		std::future_status ready = this->listeningFuture.wait_for(0ms);

		if(ready == std::future_status::ready)
		{
			// New listening thread if last one used
			std::packaged_task<void()> listener(std::thread(&Threaded_TCPListener::listenForClients);

			this->listeningFuture = listener.get_future();

			std::thread(std::move(listener), this);
		}
	}

	// Read from all clients
	std::vector<std::thread> threads;
	threads.reserve(clients.size());

	// Recv from client sockets
	for (int sock : this->clients)
	{
		threads.emplace_back(std::thread(&Threaded_TCPListener::receiveFromSocket, this, sock));
	}

	// Wait for all threads to finish
	for(std::thread& t : threads)
	{
		t.join();
	}

	return 0;
}

Threaded_TCPListener::~Threaded_TCPListener()
{
	// Remove listener socket and close it.
	closesocket(this->socket);

	// Remove all client sockets and close them
	while(!this->clients.empty())
	{
		std::unordered_set<int>::iterator it = clients.begin();

		int socketToClose = *it;
		this->clients.erase(*it);
		closesocket(socketToClose);
	}

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

void Threaded_TCPListener::sendMessageToClient(int clientSocket, const char * msg, int length)
{
	send(clientSocket, msg, length, 0);

	return;
}

void Threaded_TCPListener::broadcastToClients(int senderSocket, const char * msg, int length)
{
	std::vector<std::thread> threads;
	threads.reserve(clients.size());

	// Iterate over all clients
	for (int sendSock : this->clients)
	{
		if(sendSock != senderSocket)
			threads.emplace_back(std::thread(&Threaded_TCPListener::sendMessageToClient, this,sendSock, msg, length));
	}

	// Wait for all threads to finish
	for(std::thread& t : threads)
		t.join();

	return;
}

void Threaded_TCPListener::receiveFromSocket(int receivingSocket)
{
	// Byte storage
	char buff[MAX_BUFF_SIZE];

	// Clear buff
	memset(buff, 0, sizeof(buff));

	// Receive msg
	int bytesRecvd = recv(receivingSocket, buff, MAX_BUFF_SIZE, 0);
	if(bytesRecvd <= 0)
	{
		// Close client
		this->clients.erase(receivingSocket);
		closesocket(receivingSocket);

		onClientDisconnected(receivingSocket);
	}
	else
	{
		onMessageReceived(receivingSocket, buff, bytesRecvd);
	}
}

void Threaded_TCPListener::listenForClients()
{
	int client = accept(this->socket, nullptr, nullptr);

	// Accept Error
	if(client == INVALID_SOCKET)
	{
		std::printf("Accept Err: %d\n", WSAGetLastError());
	}
	// Add client to clients queue
	else
	{
		// Add client to queue
		this->clients.emplace(client);

		// Client Connect Confirmation
		onClientConnected(client);
	}

	return;
}
