#include "EnginePCH.h"
#include "Scene.h"


Scene::Scene() 
{
	// Makes sure all entities with transforms get a constantbuffer
	m_ecsRegistry.on_construct<ecs::component::Transform>().connect<ecs::OnTransformConstruct>();

}

entt::entity Scene::CreateEntity()
{
	return m_ecsRegistry.create();
}

void Scene::DestroyEntity(entt::entity entity)
{
	m_ecsRegistry.destroy(entity);
}

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