#pragma once
#include "DoubleBuffer.h"

// Elements
#include "Element2D.h"

/*
	Defines for ease of use.
*/

/*
	WIKI:
	Handler2D is a singleton class used to render and update each element.
	It is as well used for fetching elements through their m_name string.

	Rendering:
	To render an element simply use the InsertElement and it will be
	set up for rendering.

	Managing Elements:
	To fetch any elements put into the system you will have firstly assign
	a unique name to your element and then by that unique name call
	GetElement<Element_Type>(name); function to fetch it. Make sure
	your Element_Type is matching with the destination pointer. Failed
	conversion will result in nullptr.

	Memory management:
	Any pointer put into the Handler2D system will be taken care of at the
	end of its life cycle. That means if you delete any pointer you
	put into the Handler2D it might or might not crash. Best to leave it to
	the Handler to take care of memory deallocation.

	Functions to ignore:
	Render();
	Update();
	IsRenderReady();
*/


	// class used to render and update each element.
	class Handler2D
	{
	private:

		/*
			Unordered map containing the group and name as key.
		*/
		std::map<std::string, Element2D*> m_elements;

		// Doublebuffer holding references to elements.
		DoubleBuffer<std::vector<Element2D**>> m_drawBuffers;

	public:

		Handler2D();
		~Handler2D();

		// Insert an element into the rendering system.
		void InsertElement(Element2D* element, std::string& name);

		/*
			Get an Element by its assigned name.
			nullptr will be returned if
			element couldn't be casted to Template.
		*/
		template<class T>
		T* GetElement(const std::string& element_name) const;

		// Render all elements.
		void Render();

		// Update the states of all buttons.
		void Update();

		bool IsRenderReady() const;

	};


	template<class T>
	inline T* Handler2D::GetElement(const std::string& element_name) const
	{
		if (m_elements.find(element_name) != m_elements.end())
		{
			return dynamic_cast<T*>(m_elements.at(element_name));
		}

		return nullptr;
	}
