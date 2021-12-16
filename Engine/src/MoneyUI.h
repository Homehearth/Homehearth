#pragma once
#include "Text.h"
#include "Picture.h"

namespace rtd
{
	class MoneyUI : public Element2D
	{
	private:

		std::unique_ptr<Text> m_text;
		std::unique_ptr<Picture> m_picture;
		uint32_t m_networkMoney = 0;
		uint32_t m_localMoney = 0;
		draw_text_t m_drawOpts;

		void Update();

	public:

		MoneyUI(const draw_text_t& opts);
		void SetNewMoney(const uint32_t& moneyRef);

		const uint32_t& GetNetworkMoney() const;
		const uint32_t& GetLocalMoney() const;

		// Inherited via Element2D
		virtual void Draw() override;

		virtual void OnClick() override;

		virtual void OnHover() override;

		virtual bool CheckHover() override;

		virtual ElementState CheckClick() override;

	};
}