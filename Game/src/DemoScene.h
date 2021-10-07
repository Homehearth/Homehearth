#pragma once
#include "EnginePCH.h"
#include "GameSystems.h"

class DemoScene : public SceneBuilder<DemoScene>
{
private:
	Entity m_player;
	uint32_t* m_playerID, *m_gameID;
	Client& m_client;
public:
	DemoScene(HeadlessEngine& engine, Client& client, uint32_t* playerID, uint32_t* gameID);
	
	Entity CreatePlayerEntity();
	

};