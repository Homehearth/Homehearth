#pragma once
#include "Element2D.h"
#include "Picture.h"
#include "MoneyUI.h"
#include "Border.h"

namespace rtd
{
	class ShopUI : public Element2D
	{
	private:

		std::unique_ptr<Picture> m_texture = nullptr;
		std::unique_ptr<Picture> m_signTexture = nullptr;
		std::unique_ptr<Text> m_signText = nullptr;
		std::unique_ptr<Border> m_chosenBorder = nullptr;
		draw_t m_drawOpts;
		MoneyUI* m_moneyRef = nullptr;

		bool m_buttonHovering[6] = { false };
		std::function<void()> m_functions[6] = {nullptr};
		std::function<void()> m_onHoverFunction = nullptr;

		int m_atkUpgradeCount = 0;
		int m_healthUpgradeCount = 0;
		int m_armorUpgradeCount = 0;

	public:

		ShopUI(const std::string& filePath, const draw_t& opts);
		~ShopUI() = default;

		void SetMoneyRef(MoneyUI* money);
		void SetOnPressedEvent(unsigned int index, std::function<void()> func);
		void SetOnHoverEvent(std::function<void()> func);

		// Inherited via Element2D
		virtual void Draw() override;
		virtual void OnClick() override;
		virtual void OnHover() override;
		virtual bool CheckHover() override;
		virtual ElementState CheckClick() override;
	};
}