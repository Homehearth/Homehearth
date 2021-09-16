#include "EnginePCH.h"

void resource::GResource::AddRef()
{
	m_references++;
}

void resource::GResource::DecreaseRef()
{
	m_references--;
}

const unsigned int resource::GResource::GetRef() const
{
	return m_references;
}
