#include "EnginePCH.h"
#include "Canvas.h"

Canvas::Canvas(const D2D1_COLOR_F& color, const _DRAW& opts, const std::string& name)
{
	m_color = color;
	m_drawOpts = opts;
	this->SetName(name);
	this->SetLayer(m_drawOpts.layer);
}

void Canvas::Draw()
{
	D2D1Core::DrawF(m_drawOpts, _DRAW_SHAPE(Shapes::RECTANGLE_FILLED, m_color));
}
