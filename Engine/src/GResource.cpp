#include "EnginePCH.h"

void resource::GResource::AddRef()
{
	m_references++;
}

void resource::GResource::Release()
{
	m_references--;
}

const unsigned int resource::GResource::GetRef() const
{
	return m_references;
}

void resource::SafeRelease(GResource* pointer)
{
	if (pointer == nullptr)
		return;

	pointer->Release();
	pointer = nullptr;
}
