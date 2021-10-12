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

void Scene::Update(float dt)
{
	PROFILE_FUNCTION();

	// Emit event
	publish<ESceneUpdate>(dt);
	if (!m_renderableCopies.IsSwapped())
	{
		PROFILE_SCOPE("Copy Transforms");
		m_renderableCopies[0].clear();
		m_registry.group<comp::Renderable, comp::Transform>().each([&](comp::Renderable& r, comp::Transform& t)
		{
			r.data.worldMatrix = ecs::GetMatrix(t);
			m_renderableCopies[0].push_back(r);
		});
		
		m_renderableCopies.Swap();
	}
}

void Scene::Render() 
{
	PROFILE_FUNCTION();

	
	//double start = omp_get_wtime();
	// Renders on one thread if Launch returns 1. else everything already rendered.
	const render_instructions_t inst = thread::RenderThreadHandler::Get().Launch(m_renderableCopies[1].size());
	if((inst.start | inst.stop) == 0)
	{
		ID3D11Buffer* buffers[1] =
		{
			m_publicBuffer.GetBuffer()
		};

		D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(0, 1, buffers);
		// System that renders Renderable component
		for (const auto& it : m_renderableCopies[1])
		{
			m_publicBuffer.SetData(D3D11Core::Get().DeviceContext(), it.data);
			it.model->Render();
		}
	}
	else
	{
		// Render third part of the game with immediate context

		ID3D11Buffer* buffers[1] =
		{
			m_publicBuffer.GetBuffer()
		};

		D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(0, 1, buffers);
		// System that renders Renderable component
		for (int i = inst.start; i < inst.stop; i++)
		{
			const auto& it = m_renderableCopies[1][i];
			m_publicBuffer.SetData(D3D11Core::Get().DeviceContext(), it.data);
			it.model->Render();
		}
	}

	// Run any available Command lists from worker threads.
	thread::RenderThreadHandler::ExecuteCommandLists();
	//double end = omp_get_wtime() - start;
	//std::cout << "Time: " << end << "\n";
	// Emit event
	publish<ESceneRender>();

	m_renderableCopies.ReadyForSwap();
}

const bool Scene::IsRenderReady() const
{
	return m_renderableCopies.IsSwapped();
}

Camera* Scene::GetCamera()
{
	return m_currentCamera.get();
}

DoubleBuffer<std::vector<comp::Renderable>>* Scene::GetBuffers()
{
	return &m_renderableCopies;
}
