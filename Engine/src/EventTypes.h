#pragma once
#include "Entity.h"
struct EShutdown {};

struct ESceneChange
{
	std::string newScene;
};

struct ESceneUpdate 
{
	float dt;
};

struct ESceneCollision
{
	entt::entity obj1;
	entt::entity obj2;
};

struct ESceneRender {};
