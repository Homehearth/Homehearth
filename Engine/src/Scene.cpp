#include "EnginePCH.h"
#include "Scene.h"
#include <omp.h>

Scene::Scene()
: m_IsRenderingColliders(true),
m_currentCamera(nullptr)
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
	if(!m_debugRenderableCopies.IsSwapped())
	{
		m_debugRenderableCopies[0].clear();
		m_registry.view<comp::RenderableDebug, comp::Transform>().each([&](entt::entity entity, comp::RenderableDebug& r, comp::Transform& t)
			{
				sm::Matrix mat;
				comp::BoundingOrientedBox* obb = m_registry.try_get<comp::BoundingOrientedBox>(entity);
				comp::BoundingSphere* sphere = m_registry.try_get<comp::BoundingSphere>(entity);
				if(obb != nullptr)
				{
					mat = sm::Matrix::CreateScale(obb->Extents);
				}
				else if(sphere != nullptr)
				{
					mat = sm::Matrix::CreateScale(sm::Vector3(sphere->Radius, sphere->Radius, sphere->Radius));
				}
				mat *= sm::Matrix::CreateWorld(t.position, ecs::GetForward(t), ecs::GetUp(t));
				r.data.worldMatrix = mat;
				m_debugRenderableCopies[0].push_back(r);
			});
		
		m_debugRenderableCopies.Swap();
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

void Scene::RenderDebug()
{
	if(m_IsRenderingColliders)
	{
		PROFILE_FUNCTION();

		// System that renders Renderable component

		ID3D11Buffer* buffers[1] =
		{
			m_publicBuffer.GetBuffer()
		};

		D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(0, 1, buffers);
		for (const auto& it : m_debugRenderableCopies[1])
		{
			m_publicBuffer.SetData(D3D11Core::Get().DeviceContext(), it.data);
			if (it.model)
				it.model->Render();
		}

		// Emit event
		publish<ESceneRender>();
		m_debugRenderableCopies.ReadyForSwap();
	}

}

const bool Scene::IsRenderReady() const
{
	return m_renderableCopies.IsSwapped();
}

const bool Scene::IsRenderDebugReady() const
{
	return m_debugRenderableCopies.IsSwapped();
}

Camera* Scene::GetCurrentCamera() const
{
	return m_currentCamera;
}

void Scene::ReadyForSwap()
{
	m_renderableCopies.ReadyForSwap();
	m_debugRenderableCopies.ReadyForSwap();
}

void Scene::SetCurrentCamera(Camera* pCamera)
{
	m_currentCamera = pCamera;
}

bool* Scene::GetIsRenderingColliders()
{
	return &m_IsRenderingColliders;
}

Lights* Scene::GetLights()
{
	return &m_lights;
}

DoubleBuffer<std::vector<comp::Renderable>>* Scene::GetBuffers()
{
	return &m_renderableCopies;
}

DoubleBuffer<std::vector<comp::RenderableDebug>>* Scene::GetDebugBuffers()
{
	return &m_debugRenderableCopies;
}
