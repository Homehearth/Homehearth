#pragma once
//#include <EnginePCH.h>
//#include <HeadlessEngine.h>

#include "Server.h"


class ServerGame //: public HeadlessEngine 
{
private:
	network::Server m_server;
public:
	ServerGame();

	virtual bool OnStartup();
	void Start();
};

