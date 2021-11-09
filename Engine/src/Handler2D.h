#pragma once
#include "DoubleBuffer.h"

// Elements
#include "Collection2D.h"

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
		std::map<std::string, Collection2D*> m_collections;

		// Doublebuffer holding references to elements.
		DoubleBuffer<std::vector<Element2D**>> m_drawBuffers;
		DoubleBuffer<std::vector<Collection2D**>> m_renderBuffers;

	public:

		Handler2D();
		~Handler2D();
		
		void AddElementCollection(Collection2D* collection, const char* name);
		void AddElementCollection(Collection2D* collection, std::string& name);
		Collection2D* GetCollection(const std::string& collectionName) const;

		// Render all elements.
		void Render();

		// Update the states of all buttons.
		void Update();

		bool IsRenderReady() const;
	};
