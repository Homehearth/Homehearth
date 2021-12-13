#pragma once
#include "Element2D.h"
#include "Text.h"
#include "Canvas.h"

namespace rtd
{
	class LobbyUI : public Element2D
	{
	private:

		std::unique_ptr<Text> m_lobbyID = nullptr;
		std::unique_ptr<Text> m_sceneStatus = nullptr;
		std::unique_ptr<Text> m_playerCountText = nullptr;
		std::unique_ptr<Border> m_border = nullptr;
		std::unique_ptr<Canvas> m_canvas = nullptr;

		std::function<void()> m_function = nullptr;

		draw_t m_drawOpts;
		uint32_t m_playerCount = 0;

	public:

		LobbyUI(const std::string& lobbyID, const draw_t& opts);

		void SetLobbyStatus(const std::string& lobbyStatus);
		void UpdateLobbyPlayerCount(const uint32_t& playerCount);
		void SetOnPressedEvent(std::function<void()> func);

		// Inherited via Element2D
		virtual void Draw() override;

		virtual void OnClick() override;

		virtual void OnHover() override;

		virtual bool CheckHover() override;

		virtual ElementState CheckClick() override;

	};
}