#pragma once
#include "D2D1Core.h"

/*
	Base-class for Rendering elements through D2D1
*/
class Element2D
{
private:

	// Which layer this element should be rendered at.
	unsigned int m_layer = 0;

	// Name for get() Element from Handler2D
	std::string m_name = "";

public:

	// Set the identifying name of Element.
	void SetName(const std::string& name);
	const std::string& GetName();

	Element2D();
	virtual ~Element2D();

	// Set the layer at which to draw at.
	void SetLayer(const unsigned int layer);
	const unsigned int GetLayer() const;

	virtual void Draw() = 0;
};