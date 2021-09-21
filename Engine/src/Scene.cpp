#include "EnginePCH.h"
#include "Scene.h"

Scene::Scene() 
{
	/*
	m_registry.on_construct<comp::Transform>().connect<ecs::OnTransformConstruct>();
	m_registry.on_construct<comp::Renderable>().connect<ecs::OnRenderableConstruct>();

	m_registry.on_update<comp::Transform>().connect<ecs::OnTransformUpdate>();
	*/

	m_isConstantBuffersReadySwap = true;
}

entt::registry& Scene::GetRegistry() {
	return m_registry;
}

void Scene::Update(float dt)
{
	publish<ESceneUpdate>(dt);

	if (m_isConstantBuffersReadySwap)
	{
		T_LOCK();
		auto view = m_registry.view<comp::Transform, comp::Renderable>();
		view.each([](comp::Transform& transform, comp::Renderable& renderable)
			{
				std::swap(transform.pConstantBuffer, renderable.pConstantBuffer);
			});
		T_UNLOCK();
		m_isConstantBuffersReadySwap = false;
	}
}

void Scene::Render() 
{
	auto view = m_registry.view<comp::Renderable>();
	T_LOCK();
	view.each([](comp::Renderable& renderable)
		{
			ID3D11Buffer* buffers[1] = 
			{
				renderable.pConstantBuffer->GetBuffer()
			};

			D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(0, 1, buffers);
			renderable.mesh->Render();
		});
	T_UNLOCK();
	
	m_isConstantBuffersReadySwap = true;

	publish<ESceneRender>();
}