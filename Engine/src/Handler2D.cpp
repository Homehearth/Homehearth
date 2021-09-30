#include "EnginePCH.h"

#define INSTANCE rtd::Handler2D::instance
rtd::Handler2D* INSTANCE = nullptr;

bool rtd::compare_elements(Element2D* first, Element2D* second)
{
	if (first && second)
	{
		if (first->GetLayer() < second->GetLayer())
			return true;
	}
	
	return false;
}

rtd::Handler2D::~Handler2D()
{
	for (auto elem : m_elements)
	{
		delete elem;
	}
	m_elements.clear();
}

void rtd::Handler2D::Initialize()
{
	if (!INSTANCE)
	{
		INSTANCE = new Handler2D();
	}
}

void rtd::Handler2D::Destroy()
{
	if (INSTANCE)
		delete INSTANCE;
}

void rtd::Handler2D::InsertElement(Element2D* element)
{
	INSTANCE->m_elements.insert(element);
}

void rtd::Handler2D::RemoveElement(const std::string& element_name)
{
	for (auto elem : INSTANCE->m_elements)
	{
		if (elem->GetName() == element_name)
		{
			delete elem;
			INSTANCE->m_elements.erase(elem);
		}
	}
}

void rtd::Handler2D::Render()
{
	for (auto elem : INSTANCE->m_elements)
	{
		elem->Draw();
	}
}
