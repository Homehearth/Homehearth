#pragma once
#include "RenderableObject.h"
#include "Components.h"
#include <vector>

class Backbuffer
{
private:

	Backbuffer();
	~Backbuffer();

	thread::DoubleBuffer<std::vector<comp::RenderAble>> m_buffers;

public:

	static void Initialize();
	static void Destroy();
	static Backbuffer* instance;

	static thread::DoubleBuffer<std::vector<comp::RenderAble>>* GetBuffers();
};