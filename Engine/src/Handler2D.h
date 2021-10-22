#pragma once
#include "Element2D.h"
#include "DoubleBuffer.h"

// Elements
#include "Canvas.h"
#include "Picture.h"
#include "Border.h"
#include "Button.h"
#include "Text.h"
#include "TextField.h"
#include "Slider.h"

// std
#include <set>
#include <type_traits>

/*
	Defines for ease of use.
*/

#define GET_ELEMENT(name, type) rtd::Handler2D::Get().GetElement<type>(name)

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
	Initialize();
	Destroy();
	IsRenderReady();
*/


/*
	rtd -> Render *Two* Dee [Render2D]
*/
namespace rtd
{
	// Singleton class used to render and update each element.
	class Handler2D
	{
	private:

		// Double buffer
		std::vector<Element2D*> m_elements;
		bool m_shouldClean;

		DoubleBuffer<std::vector<Element2D**>> m_drawBuffers;
		Handler2D();
		~Handler2D();

		// Internal clean up of all elements.
		void CleanHandler();

	public:

		static auto& Get()
		{
			static Handler2D instance;
			return instance;
		}

		// Insert an element into the rendering system.
		static void InsertElement(Element2D* element);

		/*
			Get an Element by its assigned name.
			nullptr will be returned if
			element couldn't be casted to Template.
		*/
		template<class T>
		static T* GetElement(const std::string& element_name);

		// Render all elements.
		static void Render();

		// Update the states of all buttons.
		static void Update();

		// Deallocate all pointers.
		static void EraseAll();

		// Remove all pointers without deallocation.
		static void RemoveAll();

		// This removes one reference from all the elements. Use this if you want safe release of all objects that has 1 reference.
		static void DereferenceAllOnce();

		// Set the visibility of all elements to boolean.
		static void SetVisibilityAll(const bool& toggle);

		static const bool IsRenderReady();

		// Clean up function to remove all elements until next draw call.
		static void Cleanup();
	};


	template<class T>
	inline T* rtd::Handler2D::GetElement(const std::string& element_name)
	{
		for (auto elem : Handler2D::Get().m_elements)
		{
			if (elem)
			{
				if (elem->GetName() == element_name)
					return dynamic_cast<T*>(elem);
			}
		}

		return nullptr;
	}
}
