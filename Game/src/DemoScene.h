#pragma once
#include <EnginePCH.h>
#include "GameSystems.h"
#include "Client.h"

void InitializePlayerEntity(Scene& scene);

void setupDemoScene(Scene& scene, Client& client);

//Camera
void SetUpCamera(Scene& scene);
void CameraUpdate(Scene& scene, float deltaTime);


