#pragma once
#include "Element2D.h"

/*
	Class used for rendering Textures with D2D1
*/
class Picture : public Element2D
{
private:

	std::shared_ptr<RBitMap> m_texture;
	draw_t m_drawOpts;

public:

	/*
		Parameters: fileName, Options
		fileName is the filename of the texture. Default path is Assets/Textures/
		Options is for position and scale.
	*/
	Picture(const std::string& fileName, const draw_t& opts);

	// Inherited via Element2D
	virtual void Draw() override;
};