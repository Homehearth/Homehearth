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

	};
}