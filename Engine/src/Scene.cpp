#include "EnginePCH.h"
#include "Scene.h"

Scene::Scene() 
{
	m_registry.on_construct<comp::Renderable>().connect<ecs::OnRenderableConstruct>();
	
	m_isConstantBuffersReadySwap = true;
}

entt::registry& Scene::GetRegistry() {
	return m_registry;
}

void Scene::Update(float dt)
{
	publish<ESceneUpdate>(dt);
}

void Scene::Render() 
{
	auto view = m_registry.view<comp::Renderable>();
	view.each([&](entt::entity e, comp::Renderable& renderable)
		{
			comp::Transform* transform = m_registry.try_get<comp::Transform>(e);
			if (transform)
			{
				sm::Matrix m = ecs::GetMatrix(*transform);
				renderable.constantBuffer.SetData(D3D11Core::Get().DeviceContext(), m);
			}

			ID3D11Buffer* buffers[1] = 
			{
				renderable.constantBuffer.GetBuffer()
			};

			D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(0, 1, buffers);
			renderable.mesh->Render();
		});
	
	publish<ESceneRender>();
}