#include "EnginePCH.h"
#include "Scene.h"
#include "Components.h"
#include "BackBuffer.h"

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

	Backbuffer::GetBuffers()->GetBuffer(0)->clear();
	auto v = m_ecsRegistry.view<comp::RenderAble>();
	v.each([](comp::RenderAble& rend) {
		//Backbuffer::GetBuffers()->GetBuffer(0)->push_back(rend);
	});
}

void Scene::Render() 
{
	for (const auto& system : m_renderSystems)
	{
		system(m_ecsRegistry);
	}
}