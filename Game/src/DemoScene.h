#pragma once
#include <EnginePCH.h>
#include <SceneBuilder.h>

#include "GameSystems.h"
#include "Client.h"

class DemoScene : public SceneBuilder<DemoScene>
{
private:
	Entity m_player;
public:
	DemoScene(HeadlessEngine& engine, Client& client);
	
	Entity CreatePlayerEntity();
	

};

