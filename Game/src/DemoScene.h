#pragma once
#include <EnginePCH.h>
#include <SceneBuilder.h>

#include "GameSystems.h"
#include "Client.h"

class DemoScene : public SceneBuilder<DemoScene>
{
private:

public:
	DemoScene(HeadlessEngine& engine, Client<network::GameMsg>& client);
	
	Entity CreatePlayerEntity();


};

