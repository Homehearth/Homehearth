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
		if (!currentKey)
		{
			return;
		}

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
				if ((InputSystem::Get().IsInShiftMode() && !InputSystem::Get().IsInCapsLock()) || (InputSystem::Get().IsInCapsLock() && !InputSystem::Get().IsInShiftMode()))
				{
					m_stringText.push_back(static_cast<char>(0x3A));
				}
				else
				{
					m_stringText.push_back(static_cast<char>(0x2E));
				}
				break;
			case VK_OEM_6:
				if ((InputSystem::Get().IsInShiftMode() && !InputSystem::Get().IsInCapsLock()) || (InputSystem::Get().IsInCapsLock() && !InputSystem::Get().IsInShiftMode()))
				{
					m_stringText.push_back(static_cast<char>(0xC5));
				}
				else
				{
					m_stringText.push_back(static_cast<char>(0xE5));
				}
				break;
			case VK_OEM_3:
				if ((InputSystem::Get().IsInShiftMode() && !InputSystem::Get().IsInCapsLock()) || (InputSystem::Get().IsInCapsLock() && !InputSystem::Get().IsInShiftMode()))
				{
					m_stringText.push_back(static_cast<char>(0xD6));
				}
				else
				{
					m_stringText.push_back(static_cast<char>(0xF6));
				}
				break;
			case VK_OEM_7:
				if ((InputSystem::Get().IsInShiftMode() && !InputSystem::Get().IsInCapsLock()) || (InputSystem::Get().IsInCapsLock() && !InputSystem::Get().IsInShiftMode()))
				{
					m_stringText.push_back(static_cast<char>(0xC4));
				}
				else
				{
					m_stringText.push_back(static_cast<char>(0xE4));
				}
				break;
			case VK_NUMPAD0:
				m_stringText.push_back(static_cast<char>(0x30));
				break;
			case VK_NUMPAD1:
				m_stringText.push_back(static_cast<char>(0x31));
				break;
			case VK_NUMPAD2:
				m_stringText.push_back(static_cast<char>(0x32));
				break;
			case VK_NUMPAD3:
				m_stringText.push_back(static_cast<char>(0x33));
				break;
			case VK_NUMPAD4:
				m_stringText.push_back(static_cast<char>(0x34));
				break;
			case VK_NUMPAD5:
				m_stringText.push_back(static_cast<char>(0x35));
				break;
			case VK_NUMPAD6:
				m_stringText.push_back(static_cast<char>(0x36));
				break;
			case VK_NUMPAD7:
				m_stringText.push_back(static_cast<char>(0x37));
				break;
			case VK_NUMPAD8:
				m_stringText.push_back(static_cast<char>(0x38));
				break;
			case VK_NUMPAD9:
				m_stringText.push_back(static_cast<char>(0x39));
				break;
			case VK_CAPITAL:
				break;
			case VK_NUMLOCK:
				break;
			case VK_CONTROL:
				break;
			case VK_RETURN:
				break;
			case VK_LWIN:
				break;
			case VK_RWIN:
				break;
			case VK_MENU:
				break;
			default:
				if (InputSystem::Get().IsInCTRLMode() && *currentKey == 0x56)
				{
					m_stringText = InputSystem::Get().GetClipboard();
					if (m_stringText.length() > m_textLimit)
					{
						m_stringText = m_stringText.substr(0, m_textLimit);
					}
				}
				else
				{
					if ((InputSystem::Get().IsInShiftMode() && !InputSystem::Get().IsInCapsLock()) || (InputSystem::Get().IsInCapsLock() && !InputSystem::Get().IsInShiftMode()))
					{
						m_stringText.push_back(static_cast<char>(*currentKey));
					}
					else
					{
						m_stringText.push_back(static_cast<char>(std::tolower(static_cast<int>(*currentKey))));
					}
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

		InputSystem::Get().GetKeyFromUPQueue();
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
		//m_canvas->SetBorderColor(borderColor);
		m_canvas->SetBorderColor({ 0.88f, 0.3f, 0.42f, 1.0f });
	}
	else
	{
		m_canvas->SetBorderColor({ 0.2f, 0.07f, 0.09f, 1.0f });
		//m_canvas->SetBorderColor({ borderColor.r, borderColor.g, borderColor.b, 0.f });
	}

	m_canvas->ShowBorder();

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

void rtd::TextField::SetShape(const Shapes& shape)
{
	m_canvas->SetShape(shape);
	m_canvas->SetBorderShape(Shapes::RECTANGLE_ROUNDED_OUTLINED);
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
	m_canvas->SetBorderColor({ 0.88f, 0.3f, 0.42f, 1.0f });
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
	m_canvas->SetBorderColor(D2D1::ColorF(0.88f, 0.3f, 0.42f, 1.0f));
}

bool rtd::TextField::CheckHover()
{
	if (m_isUsed)
	{
		this->Update();
		return false;
	}

	if (InputSystem::Get().GetMousePos().x > m_opts.x_pos &&
		InputSystem::Get().GetMousePos().x < m_opts.x_pos + m_opts.x_stretch &&
		InputSystem::Get().GetMousePos().y > m_opts.y_pos &&
		InputSystem::Get().GetMousePos().y < m_opts.y_pos + m_opts.y_stretch)
	{
		return true;
	}

	if (!m_isUsed)
	{
		m_canvas->SetBorderColor({ 0.2f, 0.07f, 0.09f, 1.0f });
	}

	return false;
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
			m_canvas->SetBorderColor({ 0.88f, 0.3f, 0.42f, 1.0f });
			return ElementState::INSIDE;
		}
		else
		{
			m_canvas->SetBorderColor({ 0.2f, 0.07f, 0.09f, 1.0f });
			m_isUsed = false;
			InputSystem::Get().SetInputState(SystemState::GAME);
		}
	}
	return ElementState::NONE;
}
