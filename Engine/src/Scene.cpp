#include "EnginePCH.h"
#include "Scene.h"

Scene::Scene()
{	
	m_publicBuffer.Create(D3D11Core::Get().Device());
}

entt::registry& Scene::GetRegistry() {
	return m_registry;
}

void Scene::Update(float dt)
{
	PROFILE_FUNCTION();

	// Emit event
	publish<ESceneUpdate>(dt);
	
	{
		PROFILE_SCOPE("Copy Transforms");
		m_renderableCopies[0].clear();
		m_registry.group<comp::Renderable, comp::Transform>().each([&](entt::entity e, comp::Renderable& r, comp::Transform& t)
		{
			r.data.worldMatrix = ecs::GetMatrix(t);
			m_renderableCopies[0].push_back(r);
		});

		if (!m_renderableCopies.IsSwapped())
		{
			m_renderableCopies.Swap();
		}
	}
}

void Scene::Render() 
{
	PROFILE_FUNCTION();

	// System that renders Renderable component

	ID3D11Buffer* buffers[1] =
	{
		m_publicBuffer.GetBuffer()
	};

	D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(0, 1, buffers);

	// Todo: Divide up work for threads.

	// Renders on one thread if Launch returns 1. else everything already rendered.
	if ((bool)thread::RenderThreadHandler::Get().Launch(m_renderableCopies[1].size(), &m_renderableCopies))
	{
		for (const auto& it : m_renderableCopies[1])
		{
			m_publicBuffer.SetData(D3D11Core::Get().DeviceContext(), it.data);
			it.mesh->Render();
		}
	}
	
	// Emit event
	publish<ESceneRender>();

	m_renderableCopies.ReadyForSwap();
}

const bool Scene::IsRenderReady() const
{
	return m_renderableCopies.IsSwapped();
}

DoubleBuffer<std::vector<comp::Renderable>>* Scene::GetBuffers()
{
	return &m_renderableCopies;
}
