#include "EnginePCH.h"
#include "Scene.h"
#include <omp.h>

Scene::Scene()
{
	m_IsRenderingColliders = true;
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
		m_registry.view<comp::RenderableDebug, comp::Transform>().each([&](comp::RenderableDebug& r, comp::Transform& t)
			{
				sm::Matrix mat = sm::Matrix::CreateScale(r.scale);
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

const bool Scene::IsRenderReady() const
{
	return m_renderableCopies.IsSwapped();
}

const bool Scene::IsRenderDebugReady() const
{
	return m_debugRenderableCopies.IsSwapped();
}

void Scene::ReadyForSwap()
{
	m_renderableCopies.ReadyForSwap();
	m_debugRenderableCopies.ReadyForSwap();
}

Camera* Scene::GetCamera()
{
	return m_currentCamera.get();
}

bool* Scene::GetIsRenderingColliders()
{
	return &m_IsRenderingColliders;
}

//Add collider mesh to all entitys with an OBB & Sphere Collider
void Scene::InitRenderableColliders()
{
	this->ForEachComponent<comp::BoundingOrientedBox, comp::Transform>([&](Entity entity, comp::BoundingOrientedBox& boxCollider, comp::Transform& transform)
		{
			comp::RenderableDebug* renderableDebug = entity.AddComponent<comp::RenderableDebug>();
			renderableDebug->scale = boxCollider.Extents;
			renderableDebug->model = ResourceManager::Get().GetResource<RModel>("Cube.obj");
		});


	this->ForEachComponent<comp::BoundingSphere, comp::Transform>([&](Entity entity, comp::BoundingSphere& sphere, comp::Transform& transform)
		{
			comp::RenderableDebug* renderableDebug = entity.AddComponent<comp::RenderableDebug>();
			renderableDebug->scale = sm::Vector3(sphere.Radius, sphere.Radius, sphere.Radius);
			renderableDebug->model = ResourceManager::Get().GetResource<RModel>("Sphere.obj");
		});
}

DoubleBuffer<std::vector<comp::Renderable>>* Scene::GetBuffers()
{
	return &m_renderableCopies;
}

DoubleBuffer<std::vector<comp::RenderableDebug>>* Scene::GetDebugBuffers()
{
	return &m_debugRenderableCopies;
}
