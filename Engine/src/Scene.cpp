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
	
	// only copy if the last frame has been rendered
	if (!m_renderableCopies.IsSwapped()) {
		PROFILE_SCOPE("Copy Transforms");
		m_renderableCopies[0].clear();
		m_registry.group<comp::Renderable, comp::Transform>().each([&](entt::entity e, comp::Renderable& r, comp::Transform& t)
			{
				r.data.worldMatrix = ecs::GetMatrix(t);
				m_renderableCopies[0].push_back(r);
			});
		m_renderableCopies.Swap();
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

bool Scene::IsRenderReady() const 
{
	return m_renderableCopies.IsSwapped();
}
