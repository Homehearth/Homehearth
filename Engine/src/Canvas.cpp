#include "EnginePCH.h"
#include "Canvas.h"

Canvas::Canvas(const D2D1_COLOR_F& color, const draw_t& opts, const std::string& name)
{
	m_color = color;
	m_drawOpts = opts;
	this->SetLayer(m_drawOpts.layer);
}

void Canvas::Draw()
{
	D2D1Core::DrawF(m_drawOpts, draw_shape_t(Shapes::RECTANGLE_FILLED, m_color));
}
