#pragma once
#include "Element2D.h"
#include "Picture.h"
#include "MoneyUI.h"

namespace rtd
{
	class ShopUI : public Element2D
	{
	private:

		std::unique_ptr<Picture> m_texture = nullptr;
		std::unique_ptr<Picture> m_signTexture = nullptr;
		std::unique_ptr<Text> m_signText = nullptr;
		draw_t m_drawOpts;
		MoneyUI* m_moneyRef = nullptr;

		bool m_buttonHovering[5] = { false };
		std::function<void()> m_functions[5] = {nullptr};

	public:

		ShopUI(const std::string& filePath, const draw_t& opts);
		~ShopUI() = default;

		void SetMoneyRef(MoneyUI* money);
		void SetOnPressedEvent(unsigned int index, std::function<void()> func);

		// Inherited via Element2D
		virtual void Draw() override;
		virtual void OnClick() override;
		virtual void OnHover() override;
		virtual bool CheckHover() override;
		virtual ElementState CheckClick() override;
	};
}