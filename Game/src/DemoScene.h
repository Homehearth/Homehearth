#pragma once
#include <EnginePCH.h>
#include <Engine.h>

// ECS DEMO
// Simple Components
struct Triangle 
{
	sm::Vector2 pos;
	float size;
};
struct Velocity 
{
	sm::Vector2 vel;
	float mag;
};

//Event
struct TriangleCollisionEvent 
{
	Triangle* triangle;
	Velocity* velocity;
};


void createTriangle(Scene& scene, float size, const sm::Vector2& pos, const sm::Vector2& velSign);
void setupDemoScene(Engine& engine, Scene& scene);

