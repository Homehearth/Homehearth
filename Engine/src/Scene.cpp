#include "EnginePCH.h"
#include "Scene.h"

entt::dispatcher Scene::s_eventDispatcher;

Scene::Scene() 
{
	GetEventDispatcher().sink<InputEvent>().connect<&Scene::OnInput>(this);
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

void Scene::OnInput(const InputEvent& e) 
{
	// TODO: TEMPORARY
	if (e.key_code == VK_ESCAPE)
	{
		// Tell Engine to shutdown
		GetEventDispatcher().enqueue<EngineEvent>({ EngineEvent::Type::SHUTDOWN });
	}
}
