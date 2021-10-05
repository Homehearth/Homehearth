#include "EnginePCH.h"

Element2D::Element2D()
{

}

Element2D::~Element2D()
{

}

void Element2D::SetName(const std::string& name)
{
	m_name = name;
}

const std::string& Element2D::GetName()
{
	return m_name;
}

void Element2D::SetLayer(const unsigned int layer)
{
	m_layer = layer;
}

const unsigned int Element2D::GetLayer() const
{
	return m_layer;
}

const bool Element2D::IsVisible() const
{
	return m_isVisible;
}

const bool Element2D::IsClicked()
{
	return m_isClicked;
}

const bool Element2D::IsHovered()
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
