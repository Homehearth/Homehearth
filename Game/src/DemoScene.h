#pragma once
#include "EnginePCH.h"
#include "GameSystems.h"

class DemoScene : public SceneBuilder<DemoScene>
{
private:
	Entity m_player;
public:
	DemoScene(HeadlessEngine& engine, Client& client);
	
	Entity CreatePlayerEntity();
	

};