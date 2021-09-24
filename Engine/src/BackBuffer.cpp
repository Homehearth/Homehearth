#include "EnginePCH.h"
#include "BackBuffer.h"

/*
	Word of advice.
	If you make a static object inside the .h file of a singleton
	and you get a linker error LNK2001? Make sure to define that object
	here in the .cpp just like the instance is defined.

	Ex.
	--.H File--
	static int x;
	
	--.CPP File--
	int Backbuffer::x = 0;
*/

constexpr const size_t OBJECT_STORAGE_SIZE = 500;
#define INSTANCE Backbuffer::instance
Backbuffer* INSTANCE = nullptr;

Backbuffer::Backbuffer()
{
	std::vector<ecs::component::Renderable>* m_bPointer = nullptr;
	m_buffers.AllocateBuffers();
	m_bPointer = m_buffers.GetBuffer(0);
	m_bPointer->reserve(OBJECT_STORAGE_SIZE);
	m_bPointer = m_buffers.GetBuffer(1);
	m_bPointer->reserve(OBJECT_STORAGE_SIZE);


	m_bPointer = nullptr;
}

Backbuffer::~Backbuffer()
{
}

void Backbuffer::Initialize()
{
	if (!INSTANCE)
		INSTANCE = new Backbuffer();
}

void Backbuffer::Destroy()
{
	if (INSTANCE)
		delete INSTANCE;
}

thread::DoubleBuffer<std::vector<ecs::component::Renderable>>* Backbuffer::GetBuffers()
{
	return &INSTANCE->m_buffers;
}
