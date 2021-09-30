#pragma once
#include "Element2D.h"

// Elements
#include "Canvas.h"
#include "Picture.h"

// std
#include <set>
#include <type_traits>


/*
	rtd -> Render *Two* Dee [Render2D]
*/
namespace rtd
{
	bool compare_elements(Element2D* first, Element2D* second);

	class Handler2D
	{
	private:

		std::set<Element2D*, std::integral_constant<decltype(&compare_elements), &compare_elements>> m_elements;
		static Handler2D* instance;
		~Handler2D();

	public:

		static void Initialize();
		static void Destroy();

		// Insert an element into the rendering system.
		static void InsertElement(Element2D* element);

		// Remove an element.
		static void RemoveElement(const std::string& element_name);

		/*
			Get an Element by its assigned name.
			nullptr will be returned if
			element couldn't be casted to Template.
		*/
		template<class T>
		static T* GetElement(const std::string& element_name);

		// Render all elements.
		static void Render();
	};

	template<class T>
	inline T* Handler2D::GetElement(const std::string& element_name)
	{
		for (auto elem : m_elements)
		{
			if (elem->GetName() == element_name)
				return dynamic_cast<T*>(elem);
		}
	}
}
