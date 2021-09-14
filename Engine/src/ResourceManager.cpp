#include <EnginePCH.h>
#include "ResourceManager.h"
#include <typeinfo>

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
