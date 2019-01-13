#pragma once
#include <thread>
#include "BlockingQueue.h"

class Client
{
public:

	Client(int socket);

	~Client();

	int socket;
	BlockingQueue<const char*> messages;

	std::thread recvThread, sendThread;
};
