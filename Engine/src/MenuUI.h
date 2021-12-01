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
		std::function<void()> m_functions[3];

	public:

		MenuUI(const std::string& texturePath, const draw_t& opts);
		~MenuUI() = default;

		void SetOnPressedEvent(unsigned int index, std::function<void()> func);

		// Inherited via Element2D
		virtual void Draw() override;

		virtual void OnClick() override;

		virtual void OnHover() override;

		virtual bool CheckHover() override;

		virtual ElementState CheckClick() override;

	};
}