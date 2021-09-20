#include <EnginePCH.h>
#include "ResourceManager.h"
#include <typeinfo>

ResourceManager* ResourceManager::m_instance = nullptr;

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
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

void ResourceManager::Initialize()
{
	if (!m_instance)
	{
		m_instance = new ResourceManager();
	}
}

void ResourceManager::Destroy()
{
	if (m_instance)
		delete m_instance;
}

void ResourceManager::RemoveResource(const std::string& resource_name)
{
	//Check if the resource exists
	auto f = ResourceManager::m_instance->m_resources.find(resource_name);

	if (f != ResourceManager::m_instance->m_resources.end())
	{
		if (f->second != nullptr)
		{
			f->second->Release();
			if (f->second->GetRef() <= 0)
			{
				delete f->second;
				ResourceManager::m_instance->m_resources.erase(f);
			}
		}
	}
}

void ResourceManager::InsertResource(const std::string& resource_name, resource::GResource* resource)
{
	//Only insert resources that does not exist
	if (ResourceManager::m_instance->m_resources.find(resource_name) == ResourceManager::m_instance->m_resources.end())
	{
#ifdef _DEBUG
		LOG_INFO("RM added '%s'", resource_name.c_str());
#endif 
		resource->AddRef();
		ResourceManager::m_instance->m_resources.emplace(resource_name, resource);
	}
}
