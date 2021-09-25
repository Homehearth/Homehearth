#pragma once
#include "GResource.h"

/*
	Resourcemanager is a singleton and can be used everywhere
	* Functionality:
		* add - add a resource without using virtual create-function
		* get - will get the resource and can create it if it does not exist
		* free - remove any resources not in use
	* Limits: limited to only objects which has "GResource" as baseclass

	* Shared pointers 
		* Every resource has a shared_ptr which means that every resource
		* has a referens counter. Can only destroy a resource when none is using it

	Code example:
	* 
	* std::shared_ptr<RMaterial> material = ResourceManager::Get().AddResource<RMaterial>("testMat");
	* material.specialCreate(...);
	* 
	* std::shared_ptr<RMesh> mesh = ResourceManager::Get().GetResource<RMesh>("Cube.fbx");
	* 

	Future work (if needed):
	* If the resource manager is performaning slow - dynamic_pointer_cast
*/

class ResourceManager
{
private:
	ResourceManager() = default;
	std::unordered_map<std::string, std::shared_ptr<resource::GResource>> m_resources;

public:
	~ResourceManager() = default;
	
	//Delete copy constructors
	ResourceManager(const ResourceManager& rm) = delete;
	ResourceManager(const ResourceManager&& rm) = delete;
	ResourceManager& operator=(const ResourceManager& rm) = delete;
	ResourceManager& operator=(const ResourceManager&& rm) = delete;

	static auto& Get()
	{
		static ResourceManager s_instance;
		return s_instance;
	}

	/*
		Add the resource without virtual create function
	*/
	template <class T>
	std::shared_ptr<T> AddResource(const std::string& resource_name);

	/*
		Retrieve any resource with the name (resource_name).
		Tries to create it if it does not exist
		Use the appropiate T class when retrieving a resource.
		Failure to apply correct T class will return a nullptr.
		Ex:
		RTexture* texture = GetResource<RTexture>(resource_name);
	*/
	template <class T>
	std::shared_ptr<T> GetResource(const std::string& resource_name);

	/*
		Removes every resource in the manager
	*/
	void Destroy();

	/*
		Clearing up all the resources that is not currently being used by anyone
	*/
	void FreeResources();

};


/*
* 
*	Templates with class needes to be done in .h-file
* 
*/

template <class T>
std::shared_ptr<T> ResourceManager::AddResource(const std::string& resource_name)
{
#ifdef _DEBUG
	LOG_INFO("RM added '%s'", resource_name.c_str());
#endif 

	std::shared_ptr<T> resource = std::make_shared<T>();
	m_resources.insert({ resource_name, resource });
	return std::dynamic_pointer_cast<T>(resource);
}

template<class T>
inline std::shared_ptr<T> ResourceManager::GetResource(const std::string& resource_name)
{
	//Check if the resource exists
	auto f = m_resources.find(resource_name);
	
	//Return the resource if it exists
	if (f != m_resources.end())
	{
		//Go down the hierachy: from base (GResource) to derived (T)
		//https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast
		return std::dynamic_pointer_cast<T>(f->second);
	}
	//Create the resource
	else
	{
		std::shared_ptr<T> resource = std::make_shared<T>();
		
		if (!resource->Create(resource_name))
		{
			return nullptr;
		}
		
#ifdef _DEBUG
		LOG_INFO("RM added and created '%s'", resource_name.c_str());
#endif 
		m_resources.insert({ resource_name, resource });
		return std::dynamic_pointer_cast<T>(resource);
	}

	// Failed to find resource
	return nullptr;
}
