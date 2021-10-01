#include "EnginePCH.h"
#include "Button.h"

using namespace rtd;

Button::Button()
{

}

Button::~Button()
{
	if (m_canvas)
		delete m_canvas;
	if (m_border)
		delete m_border;
	if (m_picture)
		delete m_picture;
}

rtd::Button::Button(const std::string& fileName, const draw_t& opts, const bool border)
{
	m_picture = new Picture(fileName, opts);
	m_drawOpts = opts;
	if (border)
	{
		this->GetBorder();
	}
}

Border* rtd::Button::GetBorder()
{
	if (!m_border)
	{
		m_border = new Border(m_drawOpts);
	}
	return m_border;
}

Picture* rtd::Button::GetPicture()
{
	if (!m_picture)
	{
		m_picture = new Picture();
	}
	return m_picture;
}

Canvas* rtd::Button::GetCanvas()
{
	if (!m_canvas)
	{
		m_canvas = new Canvas();
	}
	return m_canvas;
}

const bool Button::CheckClicked() const
{
    return m_isClicked;
}

void Button::Draw()
{

	// Draw Order
	if (m_border)
		m_border->Draw();
	if (m_picture)
		m_picture->Draw();
	if (m_canvas)
		m_canvas->Draw();
}

void rtd::Button::OnHover()
{

}

const bool rtd::Button::CheckClick()
{
	m_isClicked = false;
	// Check if mouse key is pressed.
	if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
	{
		// Is within bounds?
		if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos &&
			InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width &&
			InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos &&
			InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height)
		{
			m_isClicked = true;
		}
	}

	return m_isClicked;
}

const bool rtd::Button::CheckHover()
{
	/*
	m_isHovering = false;
	// Is within bounds?
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height)
	{
		m_isHovering = true;
	}
	return m_isHovering;
	*/

	return false;
}

void Button::OnClick()
{
	std::cout << "CLICKED!\n";
}
