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

DoubleBuffer<std::vector<comp::Renderable>>& Scene::GetRenderableCopies()
{
	return m_renderableCopies;
}

dx::ConstantBuffer<basic_model_matrix_t>& Scene::GetRenderableBuffer()
{
	return m_publicBuffer;
}

const bool Scene::IsRenderReady() const
{
	return m_renderableCopies.IsSwapped();
}

DoubleBuffer<std::vector<comp::Renderable>>* Scene::GetBuffers()
{
	return &m_renderableCopies;
}
