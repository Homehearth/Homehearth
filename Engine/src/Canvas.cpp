#include "EnginePCH.h"
#include "Canvas.h"

using namespace rtd;

Canvas::Canvas(const D2D1_COLOR_F& color, const draw_t& opts)
{
	m_color = color;
	m_drawOpts = opts;
	m_border = nullptr;
	this->SetLayer(m_drawOpts.layer);
}

rtd::Canvas::Canvas(const draw_t& opts)
{
	m_drawOpts = opts;
	m_border = nullptr;
	m_color = D2D1::ColorF(1.0f, 1.0f, 1.0f);
}

rtd::Canvas::Canvas()
{
	m_border = nullptr;
	m_color = D2D1::ColorF(1.0f, 1.0f, 1.0f);
}

rtd::Canvas::~Canvas()
{

}

void rtd::Canvas::SetColor(const D2D1_COLOR_F& new_color)
{
	m_color = new_color;
}

D2D1_COLOR_F& rtd::Canvas::GetColor()
{
	return m_color;
}

Border* rtd::Canvas::GetBorder()
{
	if (!m_border)
	{
		m_border = std::make_unique<Border>(m_drawOpts);
	}
	return m_border.get();
}

void Canvas::Draw()
{
	if (m_border)
		m_border->Draw();

	D2D1Core::DrawF(m_drawOpts, draw_shape_t(Shapes::RECTANGLE_FILLED, m_color));
}

void rtd::Canvas::OnClick()
{
	m_color = D2D1::ColorF(rand() % 255, rand() % 255, rand() % 255);
}

void rtd::Canvas::OnHover()
{

}

const bool rtd::Canvas::CheckClick()
{
	// Check if mouse key is pressed.
	if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
	{
		// Is within bounds?
		if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos &&
			InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width &&
			InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos &&
			InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height)
		{
			return true;
		}
	}
	return false;
}

const bool rtd::Canvas::CheckHover()
{
	return false;
}