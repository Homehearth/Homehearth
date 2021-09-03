#pragma once
#include <unordered_map>
#include "GResource.h"

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
		template <class T>
		T GetResource(std::string resource_name) const;

	};

	template<class T>
	inline T ResourceManager::GetResource(std::string resource_name) const
	{

	}
}