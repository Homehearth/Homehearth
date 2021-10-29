#include "EnginePCH.h"

bool Element2D::IsHovered() const
{
	return m_isHovering;
}
bool Element2D::IsVisible() const
{
	return m_isVisible;
}

void Element2D::SetVisiblity(const bool& toggle)
{
	m_isVisible = toggle;
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

unsigned int Element2D::GetRef() const
{
	return m_references;
}
