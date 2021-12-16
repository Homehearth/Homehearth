#pragma once
#include "Element2D.h"
#include "Border.h"
#include "Picture.h"
#include "Text.h"
#include "Canvas.h"

namespace rtd
{
	class AbilityUI : public Element2D
	{
	private:

		std::unique_ptr<Border> m_border;
		std::unique_ptr<Picture> m_picture;
		std::unique_ptr<Text> m_cooldownText;
		std::unique_ptr<Canvas> m_overlay;

		draw_t m_opts;
		D2D1_COLOR_F m_borderColor;

		float* m_ref = nullptr;
		float* m_maxRef = nullptr;

		float m_cooldown = 0.0f;
		std::string m_buttonPress;


		void Update();
	public:

		AbilityUI(const draw_t& opts, const D2D1_COLOR_F& color, const std::string& picturePath = "");

		void SetPicture(const std::string& filePath);

		/*
			Set the visual queue for players to know which button to
			press to activate this skill.
		*/
		void SetActivateButton(const std::string& button);

		/*
			Set the border surrounding the ability to round.
		*/
		void SetRoundBorder(const float& radius);

		/*
			Set the texture for ability.
		*/
		void SetTexture(const std::string& texturePath);

		/*
			Set the reference to the cooldown object.
		*/
		void SetReference(float* ref);
		void SetMaxReference(float* ref);


		// Inherited via Element2D
		virtual void Draw() override;

		virtual void OnClick() override;

		virtual void OnHover() override;

		virtual bool CheckHover() override;

		virtual ElementState CheckClick() override;

	};
}