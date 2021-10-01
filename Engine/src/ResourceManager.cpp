#include <EnginePCH.h>
#include "ResourceManager.h"


bool ResourceManager::AddResource(const std::string& key, const std::shared_ptr<resource::GResource>& resource)
{
	bool added = false;
	if (m_resources.find(key) == m_resources.end())
	{
		m_resources.emplace(key, resource);
		added = true;
#ifdef _DEBUG
		LOG_INFO("RM added '%s'", key.c_str());
#endif 
	}
	return added;
}

void ResourceManager::Destroy()
{
	for (auto it = m_resources.begin(); it != m_resources.end(); )
	{
		it = m_resources.erase(it);
	}
	m_resources.clear();
}

void ResourceManager::FreeResources()
{
	for (auto it = m_resources.begin(); it != m_resources.end(); )
	{
		//If there is only one of this left, we free up memory and delete it
		if (it->second.use_count() <= 1)
		{
#ifdef _DEBUG
			LOG_INFO("RM removed '%s'", it->first.c_str());
#endif 
			m_resources.erase(it);
			/*
				Not the most effient for now...
				Have to be in this order as everything 
				in unordered_map is "unordered"
			*/
			it = m_resources.begin();
		}
		else
			it++;
	}
#ifdef _DEBUG
	LOG_INFO("RM cleared unused resources. Resources left: %d", (int)m_resources.size());
#endif 
}
