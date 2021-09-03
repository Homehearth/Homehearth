#include <EnginePCH.h>
#include "ResourceManager.h"
using namespace resource;
#define INSTANCE ResourceManager::instance
ResourceManager* INSTANCE = nullptr;

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
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
