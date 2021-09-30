#pragma once
#include <EnginePCH.h>
#include <Engine.h>

class Game : public Engine
{
private:
	Client m_client;

	// Inherited via Engine
	virtual bool OnStartup() override;
public:
	Game();

	void Start();
};