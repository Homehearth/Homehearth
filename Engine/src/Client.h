#pragma once
#include "network.h"

class Client :public network::client_interface<network::MessageType>
{
private:
	// REMOVE LATER
	bool key[3] = { false, false, false };
	bool old_key[3] = { false, false, false };

public:
	Client();
	virtual ~Client();

	void Update();
};