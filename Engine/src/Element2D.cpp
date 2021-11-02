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