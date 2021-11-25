#pragma once
#include "Element2D.h"
#include "Canvas.h"
#include "Text.h"

namespace rtd
{
	class Healthbar : public Element2D
	{
	private:
		std::unique_ptr<Canvas> m_backGround;
		std::unique_ptr<Canvas> m_foreGround;
		//std::unique_ptr<Text> m_healthInfo;

		float* m_points;
		Entity m_entity;
		float m_maxHealth;

		// Double buffered draw opts XDD
		DoubleBuffer<draw_t> m_drawOpts;

		// size when it is on full health.
		float m_sizeFull;

		void Update();

	public:

		Healthbar(const draw_t& drawOpts);
		~Healthbar();

		/*
			Set the health variable use when rendering healthbar.
			It will be read as a float.
		*/
		void SetHealthVariable(Entity e);

		void SetPosition(const float& x, const float& y);
		void SetStretch(const float& x, const float& y);
		const draw_t GetOpts() const;

		// Inherited via Element2D
		virtual void Draw() override;

		virtual void OnClick() override;

		virtual void OnHover() override;

		virtual bool CheckHover() override;

		virtual ElementState CheckClick() override;

	};
}