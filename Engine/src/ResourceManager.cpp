#include <EnginePCH.h>
#include "ResourceManager.h"
#include <typeinfo>
using namespace resource;
#define INSTANCE ResourceManager::instance
ResourceManager* INSTANCE = nullptr;

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	for (auto it = this->resources.begin(); it != this->resources.end(); it++)
	{
		if (it->second)
		{
			delete it->second;
		}
	}
	this->resources.clear();
}

void resource::ResourceManager::Initialize()
{
	if (!INSTANCE)
	{
		INSTANCE = new ResourceManager();
	}
}

void resource::ResourceManager::Destroy()
{
	if (INSTANCE)
		delete INSTANCE;
}

void resource::ResourceManager::InsertResource(GResource* resource, std::string resource_name)
{
	if (resource && INSTANCE)
	{
		auto f = ResourceManager::instance->resources.find(resource_name);
		if (f != ResourceManager::instance->resources.end())
		{
			INSTANCE->resources.emplace(resource_name, resource);
		}
	}
}

