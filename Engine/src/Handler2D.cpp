#include "EnginePCH.h"

Handler2D::Handler2D()
{

}

Handler2D::~Handler2D()
{
	for (auto it : m_collections)
	{
		if (it.second)
		{
			delete it.second;
		}
	}

	m_collections.clear();
}

void Handler2D::AddElementCollection(Collection2D* collection, const char* name)
{
	std::string theName(name);
	if (strlen(name) == 0)
	{
		theName.push_back((const char)m_collections.size());
	}
	m_collections.emplace(theName, collection);
}

void Handler2D::AddElementCollection(Collection2D* collection, std::string& name)
{
	if (name.size() == 0)
	{
		name.push_back((const char)m_collections.size());
	}
	m_collections.emplace(name, collection);
}

Collection2D* Handler2D::GetCollection(const std::string& collectionName) const
{
	if (m_collections.find(collectionName) != m_collections.end())
	{
		return m_collections.at(collectionName);
	}

	return nullptr;
}

void Handler2D::Render()
{
	for (size_t i = 0; i < m_renderBuffers[1].size(); i++)
	{
		Collection2D* collection = *m_renderBuffers[1][i];
		if (collection)
		{
			for (size_t j = 0; j < collection->elements.size(); j++)
			{
				if (collection->elements[j]->IsVisible())
					collection->elements[j]->Draw();
			}
		}
	}
	m_renderBuffers.ReadyForSwap();
}

void Handler2D::Update()
{
	for (auto& it : m_collections)
	{
		if (it.second)
		{
			it.second->UpdateCollection();
		}
	}

	m_renderBuffers[0].clear();
	for (auto& it : m_collections)
	{
		for (size_t i = 0; i < it.second->elements.size(); i++)
		{
			m_renderBuffers[0].push_back(&it.second);
		}
	}

	if (!m_renderBuffers.IsSwapped())
	{
		m_renderBuffers.Swap();
	}
}

bool Handler2D::IsRenderReady() const
{
	return m_renderBuffers.IsSwapped();
}
