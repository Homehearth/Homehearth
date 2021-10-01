#pragma once
#include "Element2D.h"

// Elements
#include "Canvas.h"
#include "Picture.h"
#include "Border.h"

// std
#include <set>
#include <type_traits>


/*
	rtd -> Render *Two* Dee [Render2D]
*/
namespace rtd
{
	class Handler2D
	{
	private:

		std::vector<Element2D*> m_elements;
		static Handler2D* instance;
		Handler2D();
		~Handler2D();

	public:

		static void Initialize();
		static void Destroy();

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
	};

	template<class T>
	inline T* rtd::Handler2D::GetElement(const std::string& element_name)
	{
		for (auto elem : m_elements)
		{
			if (elem->GetName() == element_name)
				return dynamic_cast<T*>(elem);
		}
	}
}
