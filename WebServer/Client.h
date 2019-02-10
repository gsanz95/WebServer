#pragma once
#include "BlockingQueue.h"

#ifndef CLIENT_H
#define CLIENT_H

struct Client
{
	Client(int sock);
	~Client();

	bool operator ==(const Client& obj) const;

	int socket;
	BlockingQueue<std::string> messages;


};

namespace std
{
	template<>
	struct hash<Client*>
	{
		size_t operator()(const Client* obj) const;
	};


}

#endif