#include "EnginePCH.h"

const bool Element2D::IsHovered() const
{
	return m_isHovering;
}

void Element2D::AddRef()
{
	m_references += 1;
}

void Element2D::Release()
{
	if (m_references == 0)
		return;

	m_references -= 1;
}

const unsigned int Element2D::GetRef() const
{
	return m_references;
}
