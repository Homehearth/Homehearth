#pragma once

#include <HeadlessEngine.h>

#include "Server.h"


class ServerGame : public HeadlessEngine 
{
private:
	Server m_server;
public:
	ServerGame();

	void InputThread();

	virtual void Run() override;

	virtual bool OnStartup();
	void Start();
};

