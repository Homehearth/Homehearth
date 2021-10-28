#include "EnginePCH.h"

Handler2D::Handler2D()
{

}

Handler2D::~Handler2D()
{
	for (auto it : m_elements)
	{
		if (it.second)
		{
			delete it.second;
			it.second = nullptr;
		}
	}

	m_elements.clear();
}

void Handler2D::InsertElement(Element2D* element, std::string& name)
{
	if (name.size() <= 0)
	{
		name.push_back((const char)m_elements.size());
	}
	m_elements.emplace(name, element);
}

void Handler2D::Render()
{
	for (int i = 0; i < (int)m_drawBuffers[1].size(); i++)
	{
		Element2D* elem = *m_drawBuffers[1][i];
		if (elem)
		{
			if(elem->IsVisible())
				elem->Draw();

			elem->Release();
		}
	}

	m_drawBuffers.ReadyForSwap();
}

void Handler2D::Update()
{
	for (auto& it : m_elements)
	{
		if (it.second)
		{
			if (it.second->IsVisible())
			{
				if (it.second->CheckClick())
					it.second->OnClick();
				if (it.second->CheckHover())
					it.second->OnHover();
			}
		}
	}

	if (!m_drawBuffers.IsSwapped())
	{
		m_drawBuffers[0].clear();
		for (auto& it : m_elements)
		{
			m_drawBuffers[0].push_back(&it.second);
			it.second->AddRef();
		}

		m_drawBuffers.Swap();
	}
}
bool Handler2D::IsRenderReady() const
{
	return m_drawBuffers.IsSwapped();
}
