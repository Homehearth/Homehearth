#pragma once
#include "Element2D.h"
#include "Picture.h"

namespace rtd
{
	class MenuUI : public Element2D
	{
	private:

		std::unique_ptr<Picture> m_texture = nullptr;
		draw_t m_drawOpts;

		bool m_buttonHovering[3] = {false};

	public:

		MenuUI(const std::string& texturePath, const draw_t& opts);
		~MenuUI() = default;

		// Inherited via Element2D
		virtual void Draw() override;

		virtual void OnClick() override;

		virtual void OnHover() override;

		virtual bool CheckHover() override;

		virtual ElementState CheckClick() override;

	};
}