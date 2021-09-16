#pragma once
#include "ThreadSyncer.h"
#include "RenderableObject.h"
#include <vector>

class Backbuffer
{
private:

	Backbuffer();
	~Backbuffer();

public:

	static void Initialize();
	static void Destroy();
	static Backbuffer* instance;

	static thread::DoubleBuffer<std::vector<RenderableObject*>> m_buffers;

	static std::vector<RenderableObject*>* GetBuffer(int p_buffer);
};