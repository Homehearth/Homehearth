#pragma once
#include <unordered_map>
#include "GResource.h"
#include "RTexture.h"

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
		static void Destroy();
		static void InsertResource(GResource* resource, std::string resource_name);

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