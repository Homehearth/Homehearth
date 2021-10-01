#include "EnginePCH.h"

#define INSTANCE rtd::Handler2D::instance
rtd::Handler2D* INSTANCE = nullptr;
std::vector<Element2D*> rtd::Handler2D::m_elements = {};

rtd::Handler2D::Handler2D()
{

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
	INSTANCE->m_elements.push_back(element);
}

void rtd::Handler2D::Render()
{
	for (auto elem : INSTANCE->m_elements)
	{
		if (elem)
		{
			if(elem->IsVisible())
				elem->Draw();
		}
	}
}

void rtd::Handler2D::Update()
{
	for (auto elem : INSTANCE->m_elements)
	{
		if (elem)
		{
			if (elem->CheckClick())
				elem->OnClick();
			if (elem->CheckHover())
				elem->OnHover();
		}
	}
}

void rtd::Handler2D::EraseAll()
{
	for (auto elem : m_elements)
	{
		delete elem;
	}
	m_elements.clear();
}

void rtd::Handler2D::RemoveAll()
{
	for (auto elem : m_elements)
	{
		elem = nullptr;
	}
	m_elements.clear();
}
