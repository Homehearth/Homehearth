#include "EnginePCH.h"
#include "Button.h"

using namespace rtd;

Button::Button()
{

}

Button::~Button()
{
}

rtd::Button::Button(const std::string& fileName, const draw_t& opts, const bool border)
{
	m_picture = std::make_unique<Picture>(fileName, opts);
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
		m_border = std::make_unique<Border>(m_drawOpts);
	}
	return m_border.get();
}

Picture* rtd::Button::GetPicture()
{
	if (!m_picture)
	{
		m_picture = std::make_unique<Picture>();
	}
	return m_picture.get();
}

Canvas* rtd::Button::GetCanvas()
{
	if (!m_canvas)
	{
		m_canvas = std::make_unique<Canvas>();
	}
	return m_canvas.get();
}

Text* rtd::Button::GetText()
{
	if (!m_text)
	{
		m_text = std::make_unique<Text>("",
			draw_text_t(
				m_drawOpts.x_pos,
				m_drawOpts.y_pos,
				m_drawOpts.width,
				m_drawOpts.height));

	}
	return m_text.get();
}

const draw_t& rtd::Button::GetOpts() const
{
	return m_drawOpts;
}

void rtd::Button::SetPosition(const float& x, const float& y)
{
	m_drawOpts.x_pos = x;
	m_drawOpts.y_pos = y;

	m_picture->UpdatePos(m_drawOpts);
}

void rtd::Button::SetScale(const float& x, const float& y)
{
	m_drawOpts.width = x;
	m_drawOpts.height = y;

	m_picture->UpdatePos(m_drawOpts);
}

void rtd::Button::AddPosition(const float& x, const float& y)
{
	m_drawOpts.x_pos += x;
	m_drawOpts.y_pos += y;

	m_picture->UpdatePos(m_drawOpts);
}

void rtd::Button::SetOnPressedEvent(const std::function<void()>& func)
{
	m_function = func;
}

void rtd::Button::SetOnHoverEvent(const std::function<void()>& func)
{
	m_hoverFunction = func;
}

bool Button::CheckClicked() const
{
	return m_isClicked;
}

void Button::Draw()
{
	// Draw Order
	if (m_border)
	{
		if (m_border->IsVisible())
			m_border->Draw();
	}
	if (m_picture)
		m_picture->Draw();
	if (m_canvas)
		m_canvas->Draw();
	if (m_text)
		m_text->Draw();

}

void rtd::Button::OnHover()
{
	if (m_hoverFunction)
	{
		m_hoverFunction();
	}
}

ElementState rtd::Button::CheckClick()
{
	m_isClicked = false;
	if (CheckHover())
	{
		// CheckCollisions if mouse key is pressed.
		if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
		{
			m_isClicked = true;
			return ElementState::INSIDE;
		}
	}
	else
	{
		// CheckCollisions if mouse key is pressed.
		if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
		{
			return ElementState::OUTSIDE;
		}
	}

	return ElementState::NONE;
}

bool rtd::Button::CheckHover()
{
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
}

void Button::OnClick()
{
	if (m_function)
	{
		audio_t audio = {};
		audio.isUnique = false;
		audio.volume = 0.3f;
		SoundHandler::Get().PlaySound("ButtonClick", audio);

		m_function();
	}
}
