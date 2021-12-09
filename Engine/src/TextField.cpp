#include "EnginePCH.h"
#include "TextField.h"
#include <conio.h>
#include <stdio.h>
using namespace rtd;

void rtd::TextField::Update()
{
	if (m_stringText.size() <= m_textLimit)
	{
		WPARAM* currentKey = InputSystem::Get().GetKeyFromDownQueue();
		if (currentKey)
		{
			if (m_stringText.size() < m_textLimit)
			{
				switch (*currentKey)
				{
				case VK_BACK:
					if (InputSystem::Get().IsInCTRLMode())
					{
						m_stringText.clear();
					}
					else if (m_stringText.size() > 0)
					{
						m_stringText.pop_back();
					}
					break;
				case VK_OEM_PERIOD:
					m_stringText.push_back(static_cast<char>(0x2E));
					break;
				default:
					if ((InputSystem::Get().IsInShiftMode() && !InputSystem::Get().IsInCapsLock()) || (InputSystem::Get().IsInCapsLock() && !InputSystem::Get().IsInShiftMode()))
					{
						m_stringText.push_back(static_cast<char>(*currentKey));
					}
					else
					{
						m_stringText.push_back(static_cast<char>(std::tolower(*currentKey)));
					}
					break;
				}
			}
			else
			{
				if (*currentKey == VK_BACK)
				{
					if (m_stringText.size() > 0)
					{
						m_stringText.pop_back();
					}
				}
			}
			currentKey = nullptr;
		}
		currentKey = InputSystem::Get().GetKeyFromUPQueue();

	}


	// Update the text
	m_text->SetText(m_stringText);
}

rtd::TextField::TextField(const draw_text_t& opts, size_t textLimit, bool isUsed, D2D1_COLOR_F borderColor)
	:m_isUsed(isUsed), m_textLimit(textLimit)
{
	m_opts = opts;
	m_text = std::make_unique<Text>("", m_opts);
	m_canvas = std::make_unique<Canvas>(D2D1_COLOR_F({ 0.8f, 0.2f, 0.3f, 1.0f }), draw_t(m_opts.x_pos, m_opts.y_pos, m_opts.x_stretch, m_opts.y_stretch));
	if (m_isUsed)
	{
		m_canvas->SetBorderColor(borderColor);
	}
	else
	{
		m_canvas->SetBorderColor({ borderColor.r, borderColor.g, borderColor.b, 0.f });
	}

	m_canvas->SetBorderThickness(LineWidth::THICC);
	m_canvas->SetBorderShape(Shapes::RECTANGLE_OUTLINED);

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
			InputSystem::Get().SetInputState(SystemState::TEXTFIELD);
			return ElementState::INSIDE;
		}
		else
		{
			m_isUsed = false;
			InputSystem::Get().SetInputState(SystemState::GAME);
			m_canvas->HideBorder();
		}
	}
	return ElementState::NONE;
}
