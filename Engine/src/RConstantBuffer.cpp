#include "EnginePCH.h"
#include "RConstantBuffer.h"

RConstantBuffer::RConstantBuffer()
    :m_buffer(nullptr)
{

}

const bool RConstantBuffer::GetBuffer(ID3D11Buffer** p_pointer)
{
    p_pointer = &m_buffer;
    if (m_buffer)
        return true;
    else
        return false;
}

bool RConstantBuffer::Create(const std::string& filename)
{
    return false;
}
