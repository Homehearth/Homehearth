#include "EnginePCH.h"
#include "Scene.h"

Scene::Scene() 
{
	m_registry.on_construct<comp::Renderable>().connect<ecs::OnRenderableConstruct>();
}

entt::registry& Scene::GetRegistry() {
	return m_registry;
}

void Scene::Update(float dt)
{
	PROFILE_FUNCTION();
	// Emit event
	publish<ESceneUpdate>(dt);
	
	// only copy if the last frame has been rendered
	
	{
		PROFILE_SCOPE("Copy Transforms");
		m_registry.view<comp::Transform>().each([&](entt::entity e, comp::Transform& t) 
			{
				m_transformCopies[0][e] = t;
			});
	}

	if (!m_transformCopies.IsSwapped()) {
		m_transformCopies.Swap();
	}
}

void Scene::Render() 
{
	PROFILE_FUNCTION();
	// System that renders Renderable component
	auto view = m_registry.view<comp::Renderable>();
	{
		PROFILE_SCOPE("Render Renderable");
		view.each([&](entt::entity e, comp::Renderable& renderable)
			{
				if (m_transformCopies[1].find(e) != m_transformCopies[1].end())
				{
					comp::Transform transform = m_transformCopies[1].at(e);
					sm::Matrix m = ecs::GetMatrix(transform);
					renderable.constantBuffer.SetData(D3D11Core::Get().DeviceContext(), m);
				}

				ID3D11Buffer* buffers[1] = 
				{
					renderable.constantBuffer.GetBuffer()
				};

				D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(0, 1, buffers);
				renderable.mesh->Render();
			});
	}
	
	// Emit event
	publish<ESceneRender>();

	m_transformCopies.ReadyForSwap();
}

bool Scene::IsRenderReady() const 
{
	return m_transformCopies.IsSwapped();
}
