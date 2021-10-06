#pragma once
#include "GResource.h"

/*
	Resourcemanager is a singleton and can be used everywhere

	----Shared pointers---- 
	* Every resource has a shared_ptr which means that every resource
	* has a referens counter. Can only destroy a resource when none is using it.
	* shared_ptr's dies when getting out of scope


	----Code example----
	add - resource get added to manager
	* std::shared_ptr<RMaterial> material = std::make_shared<RMaterial>();
	* do something with it: material.Create() or material.Whatever()
	* ResourceManager::Get().AddResource("testMat", material);
	 
	get - resource will be returned if found. If not it will try to create it
		  if create is succesfull, return resource, otherwise return no resource
	* std::shared_ptr<RModel> mesh = ResourceManager::Get().GetResource<RModel>("Cube.fbx");
	
	free - remove any resources that is not in use
	* ResourceManager::Get().FreeResources();


	----Future work (if needed)----
	* If the resource manager is performaning slow - dynamic_pointer_cast
	* Optimize freeresources
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
		Add a resource to the manager
		Returns true on success and false if it already exist
	*/
	bool AddResource(const std::string& key, const std::shared_ptr<resource::GResource>& resource);

	/*
		Retrieve any resource with the name (key).
		Creates a resource if it does not exist.
		Uses the GResource::Create()-function 
		Uses the appropiate T class when retrieving a resource.
		Returns a shared_ptr that is nullptr if it failed to create
	*/
	template <class T>
	std::shared_ptr<T> GetResource(const std::string& key);

	/*
		Removes every resource from the the manager.
		Each resource that is still being used on other places will
		keep living and get destroyed when all is gone
	*/
	void Destroy();

	/*
		Clearing up all the resources that is not currently being used by anyone
	*/
	void FreeResources();

};


/*
* 
*	Template with class needes to be done in .h-file
* 
*/
template<class T>
inline std::shared_ptr<T> ResourceManager::GetResource(const std::string& key)
{
	//Check if the resource exists
	auto f = m_resources.find(key);
	
	//Return the resource if it exists
	if (f != m_resources.end())
	{
		//Go down the hierachy: from base (GResource) to derived (T)
		//https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast
		return std::dynamic_pointer_cast<T>(f->second);
	}
	//Create a new resource of this type
	else
	{
		std::shared_ptr<T> resource = std::make_shared<T>();

		if (resource->Create(key))
		{
#ifdef _DEBUG
			LOG_INFO("RM added '%s' and created", key.c_str());
#endif
			m_resources.emplace(key, resource);
			return std::dynamic_pointer_cast<T>(resource);
		}
		else
		{
#ifdef _DEBUG
			LOG_WARNING("RM failed to create '%s'", key.c_str());
#endif
			return std::shared_ptr<T>(nullptr);
		}
	}
}
