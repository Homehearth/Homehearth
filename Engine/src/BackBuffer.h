#pragma once
#include "Components.h"
#include <vector>

class Backbuffer
{
private:

	Backbuffer();
	~Backbuffer();

	thread::DoubleBuffer<std::vector<comp::Renderable>> m_buffers;

public:

	static void Initialize();
	static void Destroy();
	static Backbuffer* instance;

	static thread::DoubleBuffer<std::vector<ecs::component::Renderable>>* GetBuffers();
};