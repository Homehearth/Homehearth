#include "EnginePCH.h"
#include "BackBuffer.h"

constexpr const size_t OBJECT_STORAGE_SIZE = 500;
#define INSTANCE Backbuffer::instance
Backbuffer* INSTANCE = nullptr;

Backbuffer::Backbuffer()
{
	std::vector<RenderableObject*>* m_bPointer = nullptr;
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

std::vector<RenderableObject*>* Backbuffer::GetBuffer(int p_buffer)
{
	std::vector<RenderableObject*>* m_bPointer = Backbuffer::m_buffers.GetBuffer(std::move(p_buffer));
	return m_bPointer;
}
