#pragma once
#include "Entity.h"
#include "Components.h"

struct EShutdown {};

struct ESceneChange
{
	std::string newScene;
};

struct ESceneStart {};

struct ESceneUpdate 
{
	float dt;
};

struct ESceneCollision
{
	Entity obj1;
	Entity obj2;
	float dt;
};

struct ESceneCallWaveSystem
{
	float dt;
};

struct ESceneRender {};

struct EComponentUpdated 
{
	Entity entity;
	ecs::Component component;
};
