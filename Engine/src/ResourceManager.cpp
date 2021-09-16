#include <EnginePCH.h>
#include "ResourceManager.h"
#include <typeinfo>

#define INSTANCE ResourceManager::m_instance
ResourceManager* ResourceManager::m_instance = nullptr;

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	std::cout << "Number of items: " << m_resources.size() << std::endl;

	for (auto it = m_resources.begin(); it != m_resources.end(); it++)
	{
		if (it->second)
		{
			std::cout << "Deleting: " << it->first.c_str() << std::endl;
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
			f->second->DecreaseRef();
			if (f->second->GetRef() <= 0)
			{
#ifdef _DEBUG
				std::cout << "Object removed: " << f->first << "\n";
#endif
				delete f->second;
				INSTANCE->m_resources.erase(f);
			}
		}
	}
}

void ResourceManager::InsertResource(const std::string& resource_name, resource::GResource* resource)
{
	//Check if the resource exists
	auto f = ResourceManager::m_instance->m_resources.find(resource_name);
	if (f != ResourceManager::m_instance->m_resources.end())
	{
		resource->AddRef();
		INSTANCE->m_resources.emplace(resource_name, resource);
	}
	else
	{
		LOG_WARNING("[ResourceManager] A resource with the name: '" + resource_name + "' already exists.");
	}
}
