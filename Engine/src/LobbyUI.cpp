#include "EnginePCH.h"
#include "LobbyUI.h"

rtd::LobbyUI::LobbyUI(const std::string& lobbyID, const draw_t& opts)
{
	m_lobbyID = std::make_unique<Text>(lobbyID, draw_t(opts.x_pos, opts.y_pos, opts.width * 0.33f, opts.height));
	m_sceneStatus = std::make_unique<Text>("In Lobby", draw_t(opts.x_pos + (opts.width * 0.66f), opts.y_pos, opts.width * 0.33f, opts.height));

	m_playerCountText = std::make_unique<Text>("0/4", draw_t(opts.x_pos + (opts.width * 0.33f), opts.y_pos, opts.width * 0.33f, opts.height));
	m_border = std::make_unique<Border>(opts);
	m_canvas = std::make_unique<Canvas>(opts);
	m_canvas->SetColor({ 178.f / 255.f, 44.f / 255.f, 65.f / 255.f, 1.0f });
	m_canvas->SetShape(Shapes::RECTANGLE_ROUNDED);
	m_drawOpts = opts;
}

void rtd::LobbyUI::SetLobbyStatus(bool isInGame)
{
	if (isInGame)
	{
		m_sceneStatus->SetText("In Game");
	}
	else
	{
		m_sceneStatus->SetText("In Lobby");
	}
}

void rtd::LobbyUI::UpdateLobbyPlayerCount(const uint32_t& playerCount)
{
	m_playerCount = playerCount;
	m_playerCountText->SetText(std::to_string(m_playerCount) + "/4");
}

void rtd::LobbyUI::SetOnPressedEvent(std::function<void()> func)
{
	m_function = func;
}

void rtd::LobbyUI::Draw()
{
	if (m_canvas)
		m_canvas->Draw();

	if (m_lobbyID)
		m_lobbyID->Draw();
	if (m_sceneStatus)
		m_sceneStatus->Draw();
	if (m_playerCountText)
		m_playerCountText->Draw();

	if (m_border)
		m_border->Draw();

}

void rtd::LobbyUI::OnClick()
{
	if (m_function)
		m_function();
}

void rtd::LobbyUI::OnHover()
{
	m_border->SetColor(D2D1::ColorF(0.88f, 0.3f, 0.42f, 1.0f));
}

bool rtd::LobbyUI::CheckHover()
{
	// Is within bounds?
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height)
	{
		return true;
	}

	m_border->SetColor(D2D1::ColorF(0.2f, 0.07f, 0.09f, 1.0f));
	return false;
}

ElementState rtd::LobbyUI::CheckClick()
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
