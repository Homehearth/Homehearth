#include "EnginePCH.h"

#define INSTANCE BackBuffer::instance
BackBuffer* INSTANCE = nullptr;

void BackBuffer::Initialize()
{
    if (!INSTANCE)
    {
        INSTANCE = new BackBuffer();
    }
}

void BackBuffer::Destroy()
{
    if (INSTANCE)
        delete INSTANCE;
}

DoubleBuffer<std::unordered_map<entt::entity, comp::Renderable>>* BackBuffer::GetBuffers()
{
    return &INSTANCE->m_buffers;
}
