#include "EnginePCH.h"
#include "Scene.h"
#include <omp.h>
Scene::Scene()
{	
	m_publicBuffer.Create(D3D11Core::Get().Device());
	thread::RenderThreadHandler::Get().SetObjectsBuffer(&m_renderableCopies);
}

Entity Scene::CreateEntity()
{
	return Entity(m_registry);
}

entt::basic_registry<entt::entity>* Scene::GetRegistry()
{
	return &this->m_registry;
}

void Scene::Update(float dt)
{
	PROFILE_FUNCTION();

	// Emit event
	publish<ESceneUpdate>(dt);
	{
		PROFILE_SCOPE("Copy Transforms");
		m_renderableCopies[0].clear();
		m_registry.group<comp::Renderable, comp::Transform>().each([&](comp::Renderable& r, comp::Transform& t)
		{
			r.data.worldMatrix = ecs::GetMatrix(t);
			m_renderableCopies[0].push_back(r);
		});

		m_renderableCopies.Swap(0, 1);
	}
}

void Scene::Render()
{
	PROFILE_FUNCTION();
	// Divides up work between threads.
	const render_instructions_t inst = thread::RenderThreadHandler::Get().Launch((unsigned int)m_renderableCopies[2].size());
	if((inst.start | inst.stop) == 0)
	{
		// Render everything on same thread.
		ID3D11Buffer* const buffers[1] =
		{
			m_publicBuffer.GetBuffer()
		};

		D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(0, 1, buffers);
		// System that renders Renderable component
		for (const auto& it : m_renderableCopies[2])
		{
			m_publicBuffer.SetData(D3D11Core::Get().DeviceContext(), it.data);
			it.model->Render();
		}
	}
	else
	{
		// Render third part of the scene with immediate context
		ID3D11Buffer* const buffers[1] =
		{
			m_publicBuffer.GetBuffer()
		};

		D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(0, 1, buffers);
		// System that renders Renderable component
		for (int i = inst.start; i < inst.stop; i++)
		{
			const auto& it = m_renderableCopies[2][i];
			m_publicBuffer.SetData(D3D11Core::Get().DeviceContext(), it.data);
			it.model->Render();
		}
	}

	// Run any available Command lists from worker threads.
	thread::RenderThreadHandler::ExecuteCommandLists();

	// Emit event
	publish<ESceneRender>();
}

/*
const bool Scene::IsRenderReady() const
{
	return m_renderableCopies.IsSwapped();
}
*/

void Scene::ReadyForSwap()
{
	if(m_renderableCopies.IsSwapped())
	m_renderableCopies.Swap(1, 2);
}


Camera* Scene::GetCamera()
{
	return m_currentCamera.get();
}

/*
DoubleBuffer<std::vector<comp::Renderable>>* Scene::GetBuffers()
{
	return &m_renderableCopies;
}
*/
