#include "EnginePCH.h"
#include "Scene.h"

void Scene::OnRenderableDestroy(entt::registry& reg, entt::entity e)
{
	m_renderableCopies[0].erase(e);
	m_destroyedEntityMutex.lock();
	m_renderableCopies[1].erase(e);
	m_destroyedEntityMutex.unlock();
}

Scene::Scene()
{
	m_registry.on_destroy<comp::Renderable>().connect<&Scene::OnRenderableDestroy>(this);

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
		m_registry.view<comp::Transform, comp::Renderable>().each([&](entt::entity e, comp::Transform& t, comp::Renderable& r) 
			{
				r.transformCopy = t;
				m_renderableCopies[0][e] = r;
			});
		m_renderableCopies.Swap();
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
	m_destroyedEntityMutex.lock();
	for (auto& it = m_renderableCopies[1].begin(); it != m_renderableCopies[1].end(); it++)
	{
		m_publicBuffer.SetData(D3D11Core::Get().DeviceContext(), ecs::GetMatrix(it->second.transformCopy));
		it->second.mesh->Render();	
	}
	m_destroyedEntityMutex.unlock();
	
	// Emit event
	publish<ESceneRender>();

	m_renderableCopies.ReadyForSwap();
}

bool Scene::IsRenderReady() const 
{
	return m_renderableCopies.IsSwapped();
}
