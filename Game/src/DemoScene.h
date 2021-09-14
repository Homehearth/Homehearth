#pragma once
#include <EnginePCH.h>
#include <Engine.h>

// ECS DEMO
// Simple Components
struct Triangle 
{
	float pos[2];
	float size[2];
};
struct Velocity 
{
	float vel[2];
	float mag;
};

//Event
struct TriangleCollisionEvent 
{
	Triangle* triangle;
	Velocity* velocity;
};


void createTriangle(Scene& scene, float size, const float pos[2], const int velSign[2]);
void setupDemoScene(Engine& engine, Scene& scene);

