#include "EnginePCH.h"
#include "Scene.h"

Scene::Scene() 
{
	m_registry.on_construct<comp::Renderable>().connect<ecs::OnRenderableConstruct>();
	m_hasRendered = true;
}

entt::registry& Scene::GetRegistry() {
	return m_registry;
}

void Scene::Update(float dt)
{
	// Emit event
	publish<ESceneUpdate>(dt);
	
	// only copy if the last frame has been rendered
	if (m_hasRendered)
	{
		m_registry.view<comp::Transform>().each([&](entt::entity e, comp::Transform& t) 
			{
				m_transformCopies[e] = t;
			});
		m_hasRendered = false;
	}
}

void Scene::Render() 
{

	while (m_hasRendered); // makes sure render thread is not faster than update thread

	// System that renders Renderable component
	auto view = m_registry.view<comp::Renderable>();
	view.each([&](entt::entity e, comp::Renderable& renderable)
		{
			if (m_transformCopies.find(e) != m_transformCopies.end())
			{
				comp::Transform transform = m_transformCopies.at(e);
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
	m_hasRendered = true;

	// Emit event
	publish<ESceneRender>();
}