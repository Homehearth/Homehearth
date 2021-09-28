#pragma once

struct EShutdown {};

struct ESceneChange
{
	std::string newScene;
};

struct ESceneUpdate 
{
	float dt;
};

struct ESceneRender {};
