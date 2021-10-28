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

		std::shared_ptr<RBitMap> m_texture;
		draw_t m_drawOpts;
		std::unique_ptr<Border> m_border;

	public:

		/*
			Parameters: fileName, Options
			fileName is the filename of the texture. Default path is Assets/Textures/
			Options is for position and scale.
		*/
		Picture(const std::string& fileName, const draw_t& opts);
		Picture();
		virtual ~Picture() override;

		/*
			First time this function is called a border will be created.
		*/
		Border* GetBorder();

		// remove the border and deallocate the space used for it.
		void RemoveBorder();

		// set the texture of the picture.
		void SetTexture(const std::string& fileName);

		// Update the position.
		void UpdatePos(const draw_t& new_pos);

		// Inherited via Element2D
		virtual void Draw() override;
		virtual void OnClick() override;
		virtual void OnHover() override;
		virtual const bool CheckClick() override;

		// Inherited via Element2D
		virtual const bool CheckHover() override;
	};
}