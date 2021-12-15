#pragma once
#include "Element2D.h"
#include "Border.h"

namespace rtd
{
	/*
		Class used for rendering Textures with D2D1
	*/
	class Picture : public Element2D
	{
	private:

		draw_t m_drawOpts;
		std::unique_ptr<Border> m_border;
		DoubleBuffer<std::shared_ptr<RBitMap>> m_texture;
		FLOAT m_opacity = 1.0f;
		std::function<void()> m_hoverFunction = nullptr;

	public:

		/*
			Parameters: fileName, Options
			fileName is the filename of the texture. Default path is Assets/Textures/
			Options is for position and scale.
		*/
		Picture(const std::string& fileName, const draw_t& opts);
		Picture();
		virtual ~Picture() override;
		const draw_t& GetOpts() const;
		/*
			First time this function is called a border will be created.
		*/
		Border* GetBorder();

		// remove the border and deallocate the space used for it.
		void RemoveBorder();

		void SetOnHoverEvent(const std::function<void()>& func);

		// set the texture of the picture.
		void SetTexture(const std::string& fileName);

		// Update the position.
		void UpdatePos(const draw_t& new_pos);

		void SetPosition(const FLOAT& x, const FLOAT& y);

		void SetOpacity(const FLOAT& opacity);

		// Inherited via Element2D
		virtual void Draw() override;
		virtual void OnClick() override;
		virtual void OnHover() override;
		virtual ElementState CheckClick() override;

		// Inherited via Element2D
		virtual bool CheckHover() override;
	};
}