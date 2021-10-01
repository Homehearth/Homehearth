#include "EnginePCH.h"
#include "Canvas.h"

Canvas::Canvas(const D2D1_COLOR_F& color, const _DRAW_T& opts, const std::string& name)
{
	m_color = color;
	m_drawOpts = opts;
	this->SetLayer(m_drawOpts.layer);
}

void Canvas::Draw()
{
	D2D1Core::DrawF(m_drawOpts, _DRAW_SHAPE_T(Shapes::RECTANGLE_FILLED, m_color));
}
