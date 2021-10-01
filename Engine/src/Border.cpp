#include "EnginePCH.h"
#include "Border.h"

rtd::Border::Border()
{
	// Default Construction.
}

rtd::Border::Border(const draw_t& opts)
{
	m_opts = opts;
	m_borderOpts.color = D2D1::ColorF(1.0f, 1.0f, 1.0f);
	m_borderOpts.shape = Shapes::RECTANGLE_OUTLINED;
}

rtd::Border::Border(const draw_shape_t& shape, const draw_t& opts)
{
	m_borderOpts = shape;
	m_opts = opts;
}

void rtd::Border::UpdatePos(const draw_t& new_opts)
{
	m_opts = new_opts;
}

void rtd::Border::SetShape(const draw_shape_t& new_shape)
{
	m_borderOpts = new_shape;
}

void rtd::Border::Draw()
{
	D2D1Core::DrawF(m_opts, m_borderOpts);
}

const bool rtd::Border::IsClicked()
{
	return false;
}

void rtd::Border::OnClick()
{
}
