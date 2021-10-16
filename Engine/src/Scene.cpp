#include "EnginePCH.h"
#include "Scene.h"
#include <omp.h>

Scene::Scene()
	: m_currentCamera(nullptr)
{	
	m_publicBuffer.Create(D3D11Core::Get().Device());
	thread::RenderThreadHandler::Get().SetObjectsBuffer(&m_renderableCopies);
	m_defaultCamera.Initialize(sm::Vector3(0, 0, 0), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0), sm::Vector2(1000, 1000), CAMERATYPE::DEFAULT);
	m_currentCamera = &m_defaultCamera;
}

void Scene::Update(float dt)
{
	PROFILE_FUNCTION();

	// Emit event
	BasicScene::Update(dt);

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

	// Divides up work between threads.
	const render_instructions_t inst = thread::RenderThreadHandler::Get().Launch(m_renderableCopies[1].size());
	if((inst.start | inst.stop) == 0)
	{
		// Render everything on same thread.
		ID3D11Buffer* const buffers[1] =
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
		// Render third part of the scene with immediate context
		ID3D11Buffer* const buffers[1] =
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

	// Emit event
	publish<ESceneRender>();
}

const bool Scene::IsRenderReady() const
{
	return m_renderableCopies.IsSwapped();
}

Camera* Scene::GetCurrentCamera() const
{
	return m_currentCamera;
}

void Scene::ReadyForSwap()
{
	m_renderableCopies.ReadyForSwap();
}

void Scene::SetCurrentCamera(Camera* pCamera)
{
	m_currentCamera = pCamera;
}

DoubleBuffer<std::vector<comp::Renderable>>* Scene::GetBuffers()
{
	return &m_renderableCopies;
}
