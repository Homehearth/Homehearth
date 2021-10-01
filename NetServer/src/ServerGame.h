#pragma once
//#include <EnginePCH.h>
//#include <HeadlessEngine.h>

#include "Server.h"


class ServerGame //: public HeadlessEngine 
{
private:
	Server<network::MessageType> m_server;
public:
	ServerGame();

	virtual bool OnStartup();
	void Start();
};

