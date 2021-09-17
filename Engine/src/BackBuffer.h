#pragma once
#include "RenderableObject.h"
#include <vector>

class Backbuffer
{
private:

	Backbuffer();
	~Backbuffer();

	thread::DoubleBuffer<std::vector<RenderableObject*>> m_buffers;

public:

	static void Initialize();
	static void Destroy();
	static Backbuffer* instance;

	static thread::DoubleBuffer<std::vector<RenderableObject*>>* GetBuffers();
};