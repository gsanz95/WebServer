#include "Client.h"

Client::Client(int socket)
{
	this->messages = BlockingQueue<const char*>();
	this->socket = socket;
}

Client::~Client()
{

}
