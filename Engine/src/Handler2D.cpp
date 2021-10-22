#include "EnginePCH.h"

using namespace rtd;
#define INSTANCE rtd::Handler2D::Get()
//std::vector<Element2D*> rtd::Handler2D::m_elements = {};
//DoubleBuffer<std::vector<Element2D**>> rtd::Handler2D::m_drawBuffers;

rtd::Handler2D::Handler2D()
{

}

rtd::Handler2D::~Handler2D()
{
	for (auto& elem : m_elements)
	{
		delete elem;
	}
	m_elements.clear();
}

void rtd::Handler2D::InsertElement(Element2D* element, const std::string& groupName)
{
	INSTANCE.m_elements.push_back(element);
	INSTANCE.m_groups[groupName].push_back(element);
}

void rtd::Handler2D::Render()
{
	for (int i = 0; i < (int)INSTANCE.m_drawBuffers[1].size(); i++)
	{
		Element2D* elem = *INSTANCE.m_drawBuffers[1][i];
		if (elem)
		{
			if (elem->IsVisible())
				elem->Draw();

			elem->Release();
		}
	}

	INSTANCE.m_drawBuffers.ReadyForSwap();
}

void rtd::Handler2D::Update()
{
	bool shouldErase = false;
	for (int i = 0; i < INSTANCE.m_elements.size(); i++)
	{
		Element2D* elem = INSTANCE.m_elements[i];
		if (elem != nullptr)
		{
			if (elem->GetRef() > 0 && elem->IsVisible())
			{
				elem->Update();
			}
			else
				shouldErase = true;
		}
	}

	/*
		Cleanup before push up to render
	*/
	if (shouldErase)
	{
		INSTANCE.EraseAll();
	}

	if (!INSTANCE.m_drawBuffers.IsSwapped())
	{
		INSTANCE.m_drawBuffers[0].clear();
		for (int i = 0; i < INSTANCE.m_elements.size(); i++)
		{
			INSTANCE.m_drawBuffers[0].push_back(&INSTANCE.m_elements[i]);
			INSTANCE.m_elements[i]->AddRef();
		}

		INSTANCE.m_drawBuffers.Swap();
	}
}

void rtd::Handler2D::EraseAll()
{
	for (int i = 0; i < (int)INSTANCE.m_elements.size(); i++)
	{
		if (INSTANCE.m_elements[i]->GetRef() <= 0)
		{
			delete INSTANCE.m_elements[i];
			INSTANCE.m_elements.erase(INSTANCE.m_elements.begin() + i);
		}
	}
}

void rtd::Handler2D::RemoveAll()
{
	for (int i = 0; i < (int)INSTANCE.m_elements.size(); i++)
	{
		if (INSTANCE.m_elements[i]->GetRef() <= 0)
		{
			INSTANCE.m_elements.erase(INSTANCE.m_elements.begin() + i);
		}
	}
}

void rtd::Handler2D::DereferenceAllOnce()
{
	for (auto& elem : INSTANCE.m_elements)
	{
		if (elem->GetRef() > 0)
			elem->Release();
	}
}

void rtd::Handler2D::SetVisibilityAll(const bool& toggle)
{
	for (auto& elem : INSTANCE.m_elements)
	{
		if (elem->GetRef() > 0)
			elem->SetVisibility(toggle);
	}
}

void rtd::Handler2D::SetVisibilityGroup(const std::string& group, bool visible) 
{
	if (INSTANCE.m_groups.find(group) != INSTANCE.m_groups.end())
	{
		for (auto& elem : INSTANCE.m_groups[group])
		{
			if (elem->GetRef() > 0)
				elem->SetVisibility(visible);
		}
	}
}

const bool rtd::Handler2D::IsRenderReady()
{
	return INSTANCE.m_drawBuffers.IsSwapped();
}
