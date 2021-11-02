#pragma once
#include "Element2D.h"
#include "Canvas.h"

namespace rtd
{
	class Healthbar : public Element2D
	{
	private:

		std::unique_ptr<Canvas> m_backGround;
		std::unique_ptr<Canvas> m_foreGround;

		float* m_points;
		float m_maxHealth;

		// Current size on current health.
		draw_t m_drawOpts;

		// size when it is on full health.
		float m_sizeFull;

		void Update();

	public:

		Healthbar(void* health, const draw_t& drawOpts, const float& max_health = 100);
		~Healthbar();

		/*
			Set the health variable use when rendering healthbar.
			It will be read as a float.
		*/
		void SetHealthVariable(void* var, const float& max_health = 100);

		// Inherited via Element2D
		virtual void Draw() override;

		virtual void OnClick() override;

		virtual void OnHover() override;

		virtual bool CheckHover() override;

		virtual bool CheckClick() override;

	};
}