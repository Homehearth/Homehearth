#include <EnginePCH.h>
#include "ResourceManager.h"
#include <typeinfo>


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
	/*
		Need to recursively free resources???

		A mesh has material
		Material has textures
		
		Textures first
		Material later
		Mesh last
	*/

	std::cout << "Resources in the manager (before): " << m_resources.size() << std::endl;

	for (auto it = m_resources.begin(); it != m_resources.end(); )
	{
		//If there is only one of this left, we free up memory and delete it
		if (it->second.use_count() <= 1)
		{
#ifdef _DEBUG
			LOG_INFO("RM removed '%s'", it->first.c_str());
#endif 
			m_resources.erase(it);
			//Need to go back to start again, could have destroyed some object
			//Not efficient though...
			//Have to be a better way...
			it = m_resources.begin();
		}
		else
			it++;
	}

	std::cout << "Resources in the manager (after): " << m_resources.size() << std::endl;
}
