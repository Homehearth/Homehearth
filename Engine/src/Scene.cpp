#include "EnginePCH.h"
#include "Scene.h"

entt::dispatcher Scene::m_staticEventDispatcher;


void Scene::AddSystem(const SystemUpdateFunction& updateFunction) 
{
	m_updateSystems.push_back(updateFunction);
}

void Scene::AddRenderSystem(const SystemRenderFunction& renderFunction) 
{
	m_renderSystems.push_back(renderFunction);
}

void Scene::Update(float dt)
{
	for (const auto& system : m_updateSystems)
	{
		system(m_ecsRegistry, dt);
	}
}

void Scene::Render() 
{
	for (const auto& system : m_renderSystems)
	{
		system(m_ecsRegistry);
	}
}
