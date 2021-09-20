#pragma once
#include "GResource.h"

/*
	Discuss: 
	* namespace resource? (was used previously) 
	* Necessary/good to have?
*/

class ResourceManager
{
private:
	static ResourceManager* m_instance;
	std::unordered_map<std::string, resource::GResource*> m_resources;
	ResourceManager();
	~ResourceManager();

public:
	//Delete copy constructors
	ResourceManager(const ResourceManager& rm) = delete;
	ResourceManager(const ResourceManager&& rm) = delete;
	ResourceManager& operator=(const ResourceManager& rm) = delete;
	ResourceManager& operator=(const ResourceManager&& rm) = delete;

	static void Initialize();

	/*
		Any resources linked to the resource manager will be taken care of.
		Please do not delete any pointers placed into the resource manager.
	*/
	static void Destroy();

	/*
		Retrieve any resource with the name (resource_name).
		Use the appropiate T class when retrieving a resource.
		If the resource does not exist we shall try to create it.
		Failure to apply correct T class will return a nullptr.
		Ex:
		RTexture* texture = GetResource<RTexture>(resource_name);
	*/
	template <class T>
	static T* GetResource(const std::string& resource_name);
};

template<class T>
inline T* ResourceManager::GetResource(const std::string& resource_name)
{
	//Check if the resource exists
	auto f = ResourceManager::m_instance->m_resources.find(resource_name);

	//Resource is existing: returing it
	if (f != ResourceManager::m_instance->m_resources.end())
	{
		return dynamic_cast<T*>(f->second);
	}
	//Not existing: going to try to create it
	else
	{
		resource::GResource* resource;
		resource = new T();

		//Create the new resource and if it was a success, add it to the resources
		if (resource->Create(resource_name))
		{
			ResourceManager::m_instance->m_resources[resource_name] = resource;
			return dynamic_cast<T*>(resource);
		}
		else
		{
			delete resource;
		}
	}

	// Eventual faults, return nullptr
	return nullptr;
}
