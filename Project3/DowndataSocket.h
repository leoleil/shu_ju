#pragma once
#include "Socket.h"
class DowndataSocket :
	public Socket
{
public:
	DowndataSocket();
	~DowndataSocket();
public:
	int createReceiveServer(const int port, std::vector<message_buf>& message);
};

