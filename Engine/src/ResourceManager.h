#pragma once
#include "GResource.h"

/*
	Resourcemanager is a singleton and can be used everywhere

	----Shared pointers---- 
	* Every resource has a shared_ptr which means that every resource
	* has a referens counter. Can only destroy a resource when none is using it


	----Code example----
	add - resource get added to manager
	* std::shared_ptr<RMaterial> material = std::make_shared<RMaterial>();
	* do something with it: material.create or whatever
	* ResourceManager::Get().AddResource("testMat", material);
	 
	get - resource will be returned and created if not found
	* std::shared_ptr<RMesh> mesh = ResourceManager::Get().GetResource<RMesh>("Cube.fbx");
	
	free - remove any resources that is not in use
	* ResourceManager::Get().FreeResources();


	----Future work (if needed)----
	* If the resource manager is performaning slow - dynamic_pointer_cast
*/

/*
	New functions ideas
	* GetResource(): get the resource, or if it does not exist add it
	* Free(): remove resources not in use
	* 
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
		Retrieve any resource with the name (resource_name).
		Tries to create it if it does not exist
		Use the appropiate T class when retrieving a resource.
		Failure to apply correct T class will return a nullptr.
		Ex:
		RTexture* texture = GetResource<RTexture>(resource_name);
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
	//Create the resource
	else
	{
		std::shared_ptr<T> resource = std::make_shared<T>();
		
		if (!resource->Create(key))
		{
			return nullptr;
		}
		
#ifdef _DEBUG
		LOG_INFO("RM added and created '%s'", key.c_str());
#endif 
		m_resources.emplace(key, resource);
		return std::dynamic_pointer_cast<T>(resource);
	}

	// Failed to find resource
	return nullptr;
}
