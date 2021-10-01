#include "EnginePCH.h"

#define INSTANCE rtd::Handler2D::instance
rtd::Handler2D* INSTANCE = nullptr;

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
	INSTANCE->m_elements.push_back(element);
}

void rtd::Handler2D::Render()
{
	for (auto elem : INSTANCE->m_elements)
	{
		if(elem)
			elem->Draw();
	}
}
