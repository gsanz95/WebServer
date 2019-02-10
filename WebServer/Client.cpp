#include "Client.h"
#include <iostream>

Client::Client(int sock)
{
	this->socket = sock;
	this->messages = BlockingQueue<std::string>();
}

bool Client::operator==(const Client& obj) const
{
	printf("%d\n", this->socket);
	printf("%d\n", obj.socket);
	return this->socket == obj.socket;
}

size_t std::hash<struct Client*>::operator()(const Client* obj) const
{
	return hash<int>()(obj->socket);
}
