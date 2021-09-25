#include <EnginePCH.h>
#include "ResourceManager.h"
#include <typeinfo>


void ResourceManager::Destroy()
{
	//Erase calls destroy on every resource
	for (auto it = m_resources.begin(); it != m_resources.end(); )
	{
		std::cout << "Resource used before: " << it->second.use_count() << " for " << it->first << std::endl;
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
