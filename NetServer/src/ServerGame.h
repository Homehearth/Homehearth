#pragma once

#include <HeadlessEngine.h>

#include "Server.h"


class ServerGame : public HeadlessEngine 
{
private:
	Server m_server;
	std::thread m_inputThread;
public:
	ServerGame();

	void InputThread();

	virtual bool OnStartup() override;
	virtual void OnUserUpdate(float deltaTime) override;
	virtual void OnShutdown() override;
	
};

