#pragma once
#include "GResource.h"
#include "RTexture.h"

#define ADD_RESOURCE(type, name) resource::ResourceManager::instance->InsertResource(new type(std::string(name)), name)
#define GET_RESOURCE(type, name) resource::ResourceManager::instance->GetResource<type>(name)

namespace resource
{
	class ResourceManager
	{
	private:

		std::unordered_map<std::string, GResource*> resources;
		ResourceManager();
		~ResourceManager();

	public:
		static ResourceManager* instance;
		static void Initialize();
		/*
			Any resources linked to the resource manager will be taken care of.
			Please do not delete any pointers placed into the resource manager.
		*/
		static void Destroy();
		// Insert the resource and link it with a key name to be used to retrieve it.
		static void InsertResource(GResource* resource, std::string resource_name);

		/* 
			Retrieve any resource with the name (resource_name).
			Use the appropiate T class when retrieving a resource.
			Failure to apply correct T class will return a nullptr.
			Ex:
			RTexture* texture = GetResource<RTexture>(resource_name);
		*/
		template <class T>
		static T* GetResource(std::string resource_name);
	};

	template<class T>
	inline T* ResourceManager::GetResource(std::string resource_name)
	{
		auto f = ResourceManager::instance->resources.find(resource_name);
		if (f != ResourceManager::instance->resources.end())
		{
			return dynamic_cast<T*>(f->second);
		}
		else
		{
			return nullptr;
		}

		// Eventual faults, return nullptr
		return nullptr;
	}

}