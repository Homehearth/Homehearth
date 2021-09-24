#pragma once
#include "GResource.h"

/*
	Resourcemanager is a singleton and can be used everywhere
	Code example about how to use: 
	 
	* RMesh* testMesh = ResourceManager::Get().GetResource<RMesh>("Cube.fbx");
	* 
	* ResourceManager::Get().RemoveResource("Cube.fbx");
	* 
	* RMesh* testMesh = new RMesh();
	* if (testMesh.Create("Cube.fbx"))
	*	  ResourceManager::Get().InsertResource("Cube.fbx", testMesh);
	* 
*/

class ResourceManager
{
private:
	ResourceManager() = default;
	std::unordered_map<std::string, resource::GResource*> m_resources;

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
		Decreases the reference count toward the resource with
		name "resource_name". If this reference count is equal to 0
		then the resource will be removed from the system.
	*/
	void RemoveResource(const std::string& resource_name);

	/*
		Register a resource to the resource manager. A reference will be added
		onto the resource.
	*/
	void InsertResource(const std::string& resource_name, resource::GResource* resource);

	/*
		Retrieve any resource with the name (resource_name).
		Use the appropiate T class when retrieving a resource.
		If the resource does not exist we shall try to create it.
		Failure to apply correct T class will return a nullptr.
		Ex:
		RTexture* texture = GetResource<RTexture>(resource_name);
	*/
	template <class T>
	T* GetResource(const std::string& resource_name);

	/*
		Removes every object in the manager
	*/
	void Destroy();
};

template<class T>
inline T* ResourceManager::GetResource(const std::string& resource_name)
{
	//Check if the resource exists
	auto f = m_resources.find(resource_name);

	//Resource is existing: returing it
	if (f != m_resources.end())
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
			InsertResource(resource_name, resource);
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
