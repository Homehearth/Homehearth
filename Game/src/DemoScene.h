#pragma once
#include <EnginePCH.h>
#include "GameSystems.h"
#include "Client.h"

void InitializePlayerEntity(Scene& scene);

void setupDemoScene(Scene& scene, Client<network::MessageType>& client);

