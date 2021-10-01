#include "EnginePCH.h"
#include "Scene.h"

Scene::Scene() 
{
	m_registry.on_construct<comp::Renderable>().connect<ecs::OnRenderableConstruct>();
	m_publicBuffer.Create(D3D11Core::Get().Device());
	m_buffers = BackBuffer::GetBuffers();
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
	if (!m_buffers->IsSwapped()) {
		PROFILE_SCOPE("Copy Transforms");
		m_registry.view<comp::Transform, comp::Renderable>().each([&](entt::entity e, comp::Transform& t, comp::Renderable& r) 
			{
			comp::Renderable rend;
			rend.mesh = r.mesh;
			rend.renderForm = t;
			(*m_buffers)[0][e] = rend;
			});
		m_buffers->Swap();
	}
}

void Scene::Render() 
{
	PROFILE_FUNCTION();

	// System that renders Renderable component
	for (auto it = (*m_buffers)[1].begin(); it != (*m_buffers)[1].end(); it++)
	{
		auto* pointer = &it->second;
		if (pointer)
		{
			m_publicBuffer.SetData(D3D11Core::Get().DeviceContext(), ecs::GetMatrix(pointer->renderForm));
			ID3D11Buffer* buffers[1] =
			{
				m_publicBuffer.GetBuffer()
			};

			D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(0, 1, buffers);
			pointer->mesh->Render();
		}
	}

	/*
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
	*/
	
	// Emit event
	publish<ESceneRender>();

	m_buffers->ReadyForSwap();
}
