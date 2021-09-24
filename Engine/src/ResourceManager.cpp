#include <EnginePCH.h>
#include "ResourceManager.h"
#include <typeinfo>

void ResourceManager::RemoveResource(const std::string& resource_name)
{
	//Check if the resource exists
	auto f = m_resources.find(resource_name);

	if (f != m_resources.end())
	{
		if (f->second != nullptr)
		{
			f->second->Release();
			if (f->second->GetRef() <= 0)
			{
				delete f->second;
				m_resources.erase(f);
			}
		}
	}
}

void ResourceManager::InsertResource(const std::string& resource_name, resource::GResource* resource)
{
	//Only insert resources that does not exist
	if (m_resources.find(resource_name) == m_resources.end())
	{
#ifdef _DEBUG
		LOG_INFO("RM added '%s'", resource_name.c_str());
#endif 
		resource->AddRef();
		m_resources.emplace(resource_name, resource);
	}
}

void ResourceManager::Destroy()
{
	for (auto it = m_resources.begin(); it != m_resources.end(); it++)
	{
		if (it->second)
		{
			delete it->second;
		}
	}
	m_resources.clear();
}
