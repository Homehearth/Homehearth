#pragma once
#include "D2D1Core.h"

/*
	WIKI:
	Element2D is a base-class used for rendering 
	any element with the Handler2D.

	Get Started:
	Create your own class and inherit from Element2D.
	That's it. You're good to go... Maybe a little more.

	Draw:
	This function will be called each frame and represents what will be
	drawn on the screen. Use DrawF, DrawP, DrawT or any other draws.

	OnClick:
	Determines what will happen to the ELEMENT on user click. If you
	want something outside of the button to happen on click refer to
	IsClicked.

	CheckClick:
	Here you put your logic over how to detect any clicks from user.
	Make sure you put m_isClicked to true when element is clicked and
	m_isClicked to false when not.

	CheckHover:
	Here you put your logic over how to detect any mouse hovering
	over the element. Make sure to set m_isHovering to true or false.

	OnHover:
	Determines what will happen to the ELEMENT on mouse hover. Same as 
	before with OnClick, want something to happen outside? Refer to
	IsHover().

	IsHover:
	Returns true or false depending on if the mouse is hovering over the
	element or not.

	IsVisible:
	Set this value to true or false depending on if you want to present
	the element at the end of EndDraw().
*/

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

	unsigned int m_references = 1;

protected:

	// Set the visibilty
	bool m_isVisible = true;

	// check to see if element is clicked.
	bool m_isClicked = false;

	// check to see if element is being hovered over.
	bool m_isHovering = false;

public:

	// Set the identifying name of Element.
	void SetName(const std::string& name);
	const std::string& GetName() const;

	Element2D() = default;
	virtual ~Element2D() = default;

	// Set the layer at which to draw at.
	void SetLayer(const unsigned int layer);
	const unsigned int GetLayer() const;
	const bool IsVisible() const;

	virtual void Draw() = 0;

	/*
		Returns true if clicked, false if not.
	*/
	const bool IsClicked() const;

	/*
		Logic for what happens with the element if it is clicked.
	*/
	virtual void OnClick() = 0;

	/*
		Logic for what happens with the element if it is being hovered over.
	*/
	virtual void OnHover() = 0;

	/*
		Function used for Handler2D to call on OnHover.
		Write logic for detecting if anyone is hovering over the element.
	*/
	virtual const bool CheckHover() = 0;

	/*
		Returns true if hovered over, false if not.
	*/
	const bool IsHovered() const;

	/*
	Function used for Handler2D to call on OnClick.
	Write the logic for detecting if anyone has clicked the element.
	*/
	virtual const bool CheckClick() = 0;

	/*
		Add a reference to the element.
	*/
	void AddRef();

	/*
		Release the object.
	*/
	void Release();

	/*
		Get the amount of references to this element.
	*/
	const unsigned int GetRef() const;
};