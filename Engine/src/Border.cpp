#include "EnginePCH.h"
#include "Border.h"

rtd::Border::Border()
{
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

void rtd::Border::SetColor(const D2D1_COLOR_F& new_color)
{
	m_borderOpts.color = new_color;
}

D2D1_COLOR_F& rtd::Border::GetColor()
{
	return m_borderOpts.color;
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

void rtd::Border::OnClick()
{
}

void rtd::Border::OnHover()
{
}

bool rtd::Border::CheckClick()
{
	return false;
}

bool rtd::Border::CheckHover()
{
	return false;
}
