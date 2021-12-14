#include "EnginePCH.h"
#include "Canvas.h"

using namespace rtd;

Canvas::Canvas(const D2D1_COLOR_F& color, const draw_t& opts)
{
	m_color = color;
	m_drawOpts = opts;
	m_border = nullptr;
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

void rtd::Canvas::SetPosition(const float& x, const float& y)
{
	m_drawOpts.x_pos = x;
	m_drawOpts.y_pos = y;

	if(m_border)
		m_border->UpdatePos(m_drawOpts);
}

void rtd::Canvas::SetScale(const float& x_stretch, const float& y_stretch)
{
	m_drawOpts.width = x_stretch;
	m_drawOpts.height = y_stretch;
}

void rtd::Canvas::SetColor(const D2D1_COLOR_F& new_color)
{
	m_color = new_color;
}

D2D1_COLOR_F& rtd::Canvas::GetColor()
{
	return m_color;
}

const draw_t rtd::Canvas::GetOpts() const
{
	return m_drawOpts;
}

void rtd::Canvas::SetBorderThickness(const LineWidth& thicc)
{
	m_border->SetLineWidth(thicc);
}

void rtd::Canvas::SetBorderShape(const Shapes& shape)
{
	m_border->SetShape(shape);
}

void rtd::Canvas::SetShape(const Shapes& shape)
{
	m_shape = shape;
}

void rtd::Canvas::SetBorderColor(const D2D1_COLOR_F& new_color)
{
	if (!m_border)
	{
		m_border = std::make_unique<Border>(m_drawOpts);
	}
	m_border->SetColor(new_color);
}

void rtd::Canvas::HideBorder()
{
	m_border->SetColor({ m_border->GetColor().r, m_border->GetColor().g, m_border->GetColor().b, 0.f });
}

void rtd::Canvas::ShowBorder()
{
	m_border->SetColor({ m_border->GetColor().r, m_border->GetColor().g, m_border->GetColor().b, 1.f });
}

void rtd::Canvas::SetBorderWidth(const LineWidth& width)
{
	m_border.get()->SetLineWidth(width);
}

void Canvas::Draw()
{
	if (m_border)
	{
		m_border->Draw();
	}
	D2D1Core::DrawF(m_drawOpts, draw_shape_t(m_shape, m_color));
}

void rtd::Canvas::OnClick()
{

}

void rtd::Canvas::OnHover()
{

}

ElementState rtd::Canvas::CheckClick()
{
	// CheckCollisions if mouse key is pressed.
	if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
	{
		// Is within bounds?
		if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos &&
			InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width &&
			InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos &&
			InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height)
		{
			return ElementState::INSIDE;
		}
		else
			return ElementState::OUTSIDE;
	}

	return ElementState::NONE;
}

bool rtd::Canvas::CheckHover()
{
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height)
	{
		m_isHovering = true;
		return true;
	}
	m_isHovering = false;
	return false;
}
