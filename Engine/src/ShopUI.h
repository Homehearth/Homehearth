#pragma once
#include "Element2D.h"
#include "Picture.h"

namespace rtd
{
	class ShopUI : public Element2D
	{
	private:

		std::unique_ptr<Picture> m_texture = nullptr;
		draw_t m_drawOpts;

	public:

		ShopUI(const std::string& filePath, const draw_t& opts);
		~ShopUI() = default;

		// Inherited via Element2D
		virtual void Draw() override;
		virtual void OnClick() override;
		virtual void OnHover() override;
		virtual bool CheckHover() override;
		virtual ElementState CheckClick() override;
	};
}