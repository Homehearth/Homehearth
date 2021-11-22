#include "EnginePCH.h"
#include "TextField.h"
#include <conio.h>
#include <stdio.h>
using namespace rtd;

void rtd::TextField::Update()
{
	if (m_stringText.size() < m_textLimit)
	{
		for (int i = dx::Keyboard::Keys::D0; i <= dx::Keyboard::Keys::Z; i++)
		{
			if (InputSystem::Get().CheckKeyboardKey(static_cast<dx::Keyboard::Keys>(i), KeyState::PRESSED))
			{
				char c;
				c = static_cast<char>(i);
				if (i > 64)
				{
					if (!InputSystem::Get().CheckKeyboardKey(dx::Keyboard::Keys::LeftShift, KeyState::HELD))
					{
						c = static_cast<char>(i + 32);
					}
				}
				m_stringText.push_back(c);
			}
		}
		for (int i = dx::Keyboard::Keys::NumPad0; i <= dx::Keyboard::Keys::NumPad9; i++)
		{
			if (InputSystem::Get().CheckKeyboardKey(static_cast<dx::Keyboard::Keys>(i), KeyState::PRESSED))
			{
				const char c = static_cast<char>(i - dx::Keyboard::Keys::D0);
				m_stringText.push_back(c);
			}
		}
		// Checks '.' press
		if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::Keys::OemPeriod, KeyState::PRESSED))
		{
			const char c = static_cast<char>(0x2E);
			m_stringText.push_back(c);
		}
	}
	// Remove with the backspace
	if ((InputSystem::Get().CheckKeyboardKey(dx::Keyboard::Keys::Back, KeyState::PRESSED) && (m_stringText.length() > 0)) == 1)
	{
		m_stringText.pop_back();
	}

	if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::Keys::LeftControl, KeyState::HELD) &&
		InputSystem::Get().CheckKeyboardKey(dx::Keyboard::Keys::V, KeyState::PRESSED))
	{
		m_stringText = InputSystem::Get().GetClipboard();
		if (m_stringText.length() > m_textLimit)
		{
			m_stringText = m_stringText.substr(0, m_textLimit);
		}
	}

	// Update the text
	m_text->SetText(m_stringText);
}

rtd::TextField::TextField(const draw_text_t& opts, size_t textLimit, bool isUsed, D2D1_COLOR_F borderColor)
	:m_isUsed(isUsed), m_textLimit(textLimit)
{
	m_opts = opts;
	m_text = std::make_unique<Text>("", m_opts);
	m_canvas = std::make_unique<Canvas>(D2D1_COLOR_F({ 1.0f, 1.0f, 1.0f, 1.0f }), draw_t(m_opts.x_pos, m_opts.y_pos, m_opts.x_stretch, m_opts.y_stretch));
	if (m_isUsed)
	{
		m_canvas->SetBorderColor(borderColor);
	}
	else
	{
		m_canvas->SetBorderColor({ borderColor.r, borderColor.g, borderColor.b, 0.f });
	}

	m_finalInput = false;
	m_stringText = "";
}

void rtd::TextField::SetDescriptionText(const std::string& displayText)
{
	if (!m_infoText)
	{
		m_infoText = std::make_unique<Text>(displayText, draw_text_t(m_opts.x_pos, m_opts.y_pos - D2D1Core::GetDefaultFontSize() * 1.25f, m_opts.x_stretch, m_opts.y_stretch));

		return;
	}

	m_infoText->SetText(displayText);
}

void rtd::TextField::SetPresetText(const std::string& displayText)
{
	m_text->SetText(displayText);
	m_stringText = displayText;
}

std::string* rtd::TextField::RawGetBuffer()
{
	return &m_stringText;
}

void rtd::TextField::SetActive()
{
	m_isUsed = true;
	m_canvas->ShowBorder();
}

void rtd::TextField::Draw()
{
	if (m_canvas)
		m_canvas->Draw();
	if (m_text)
		m_text->Draw();
	if (m_infoText)
		m_infoText->Draw();
}

void rtd::TextField::OnClick()
{
	m_isUsed = true;
	m_canvas->ShowBorder();
}

void rtd::TextField::OnHover()
{
	if (m_isUsed)
		this->Update();
}

bool rtd::TextField::CheckHover()
{
	return true;
}

ElementState rtd::TextField::CheckClick()
{
	if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
	{
		// Is within bounds?
		if (InputSystem::Get().GetMousePos().x > m_opts.x_pos &&
			InputSystem::Get().GetMousePos().x < m_opts.x_pos + m_opts.x_stretch &&
			InputSystem::Get().GetMousePos().y > m_opts.y_pos &&
			InputSystem::Get().GetMousePos().y < m_opts.y_pos + m_opts.y_stretch)
		{
			return ElementState::INSIDE;
		}
		else
		{
			m_isUsed = false;
			m_canvas->HideBorder();
		}
	}
	return ElementState::NONE;
}
