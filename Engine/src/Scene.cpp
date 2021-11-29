#include "EnginePCH.h"
#include "Scene.h"
#include <omp.h>
#include "Systems.h"

Scene::Scene()
	: m_IsRenderingColliders(true)
{
	m_publicBuffer.Create(D3D11Core::Get().Device());
	m_publicDecalBuffer.Create(D3D11Core::Get().Device());
	m_ColliderHitBuffer.Create(D3D11Core::Get().Device());
	thread::RenderThreadHandler::Get().SetObjectsBuffer(&m_renderableCopies);

	m_defaultCamera = CreateEntity();
	m_defaultCamera.AddComponent<comp::Camera3D>()->camera.Initialize(sm::Vector3(0, 0, 0), sm::Vector3(0, 0, 1), sm::Vector3(0, 1, 0), sm::Vector2(1000, 1000), CAMERATYPE::DEFAULT);
	SetCurrentCameraEntity(m_defaultCamera);

	m_sky.Initialize("kiara1dawn.dds");
}

void Scene::Update(float dt)
{
	//Update all the animations that can be updated
	m_registry.view<comp::Animator>().each([&](comp::Animator& anim)
		{
			if (anim.animator && anim.updating)
				anim.animator->Update();
		});
	
	

	m_2dHandler.Update();
	PROFILE_FUNCTION();

	// Emit event
	GetCurrentCamera()->Update(dt);
	BasicScene::Update(dt);

	if (!m_renderableCopies.IsSwapped() &&
		!m_renderableAnimCopies.IsSwapped())
	{
		PROFILE_SCOPE("Copy Transforms");
		m_renderableCopies[0].clear();
		m_renderableAnimCopies[0].clear();

		m_registry.view<comp::Renderable, comp::Transform>().each([&](entt::entity entity, comp::Renderable& r, comp::Transform& t)
			{
				r.data.worldMatrix = ecs::GetMatrix(t);

				//Check if the model has an animator too
				comp::Animator* anim = m_registry.try_get<comp::Animator>(entity);
				if (anim != nullptr)
				{
					m_renderableAnimCopies[0].push_back({ r, *anim });
				}
				else
				{
					m_renderableCopies[0].push_back(r);
				}
			});
		m_renderableCopies.Swap();
		m_renderableAnimCopies.Swap();
		GetCurrentCamera()->Swap();
	}

	if (!m_debugRenderableCopies.IsSwapped())
	{
		m_debugRenderableCopies[0].clear();
		m_registry.view<comp::RenderableDebug>().each([&](entt::entity entity, comp::RenderableDebug& r)
			{
				comp::OrientedBoxCollider* obb = m_registry.try_get<comp::OrientedBoxCollider>(entity);
				comp::SphereCollider* sphere = m_registry.try_get<comp::SphereCollider>(entity);

				comp::Transform transform;
				transform.rotation = sm::Quaternion::Identity;

				if (obb != nullptr)
				{
					transform.scale = obb->Extents;
					transform.position = obb->Center;
					transform.rotation = obb->Orientation;
				}
				else if (sphere != nullptr)
				{
					transform.scale = sm::Vector3(sphere->Radius);
					transform.position = sphere->Center;
				}

				r.data.worldMatrix = ecs::GetMatrix(transform);
				m_debugRenderableCopies[0].push_back(r);
			});

		m_debugRenderableCopies.Swap();
	}
}

void Scene::Update2D()
{
	m_2dHandler.Update();
}

void Scene::Render()
{
	PROFILE_FUNCTION();

	thread::RenderThreadHandler::Get().SetObjectsBuffer(&m_renderableCopies);
	// Divides up work between threads.
	const render_instructions_t inst = thread::RenderThreadHandler::Get().Launch(static_cast<int>(m_renderableCopies[1].size()));

	ID3D11Buffer* const buffers[1] = { m_publicBuffer.GetBuffer() };
	D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(0, 1, buffers);

	// Render everything on same thread.
	if ((inst.start | inst.stop) == 0)
	{
		// System that renders Renderable component
		for (const auto& it : m_renderableCopies[1])
		{
			m_publicBuffer.SetData(D3D11Core::Get().DeviceContext(), it.data);

			if (it.model)
				it.model->Render(D3D11Core::Get().DeviceContext());
		}
	}
	// Render third part of the scene with immediate context
	else
	{
		// System that renders Renderable component
		for (int i = inst.start; i < inst.stop; i++)
		{
			const auto& it = m_renderableCopies[1][i];
			m_publicBuffer.SetData(D3D11Core::Get().DeviceContext(), it.data);

			if (it.model)
				it.model->Render(D3D11Core::Get().DeviceContext());
		}
	}

	// Run any available Command lists from worker threads.
	thread::RenderThreadHandler::ExecuteCommandLists();
}

void Scene::RenderDebug()
{
	if (m_IsRenderingColliders)
	{
		PROFILE_FUNCTION();

		// System that renders Renderable component

		ID3D11Buffer* buffers[1] =
		{
			m_publicBuffer.GetBuffer(),
		};
		ID3D11Buffer* buffer2[1] =
		{
			m_ColliderHitBuffer.GetBuffer(),
		};

		D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(0, 1, buffers);
		D3D11Core::Get().DeviceContext()->PSSetConstantBuffers(5, 1, buffer2);
		for (const auto& it : m_debugRenderableCopies[1])
		{
			m_publicBuffer.SetData(D3D11Core::Get().DeviceContext(), it.data);
			m_ColliderHitBuffer.SetData(D3D11Core::Get().DeviceContext(), it.isColliding);

			if (it.model)
				it.model->Render(D3D11Core::Get().DeviceContext());
		}
		m_debugRenderableCopies.ReadyForSwap();
	}

}

void Scene::Render2D()
{
	m_2dHandler.Render();

}

void Scene::RenderSkybox()
{
	m_sky.Render();
}

void Scene::RenderShadow(const light_t& light)
{
	if (!light.enabled) 
		return;

	for (const auto& model : m_renderableCopies[1])
	{
		if (model.isSolid && model.visible)
		{
			m_publicBuffer.SetData(D3D11Core::Get().DeviceContext(), model.data);
			ID3D11Buffer* buffer[] = { m_publicBuffer.GetBuffer() };
			D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(0, 1, buffer);
			if(model.model)
				model.model->Render(D3D11Core::Get().DeviceContext());
		}
	}
}

Skybox* Scene::GetSkybox()
{
	return &m_sky;
}

bool Scene::IsRenderReady() const
{
	return (IsRender2DReady() && IsRender3DReady() && IsRenderDebugReady());
}

void Scene::Add2DCollection(Collection2D* collection, std::string& name)
{
	m_2dHandler.AddElementCollection(collection, name);
}

void Scene::Add2DCollection(Collection2D* collection, const char* name)
{
	m_2dHandler.AddElementCollection(collection, name);
}

Collection2D* Scene::GetCollection(const std::string& name)
{
	return m_2dHandler.GetCollection(name);
}

void Scene::RenderAnimation()
{
	PROFILE_FUNCTION();

	for (auto& it : m_renderableAnimCopies[1])
	{
		m_publicBuffer.SetData(D3D11Core::Get().DeviceContext(), it.first.data);
		ID3D11Buffer* const buffer = {
			m_publicBuffer.GetBuffer()
		};
		D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(0, 1, &buffer);
		it.second.animator->Bind();
		it.first.model->Render(D3D11Core::Get().DeviceContext());
		it.second.animator->Unbind();
	}
}

bool Scene::IsRender3DReady() const
{
	return m_renderableCopies.IsSwapped() && m_renderableAnimCopies.IsSwapped();
}

bool Scene::IsRenderDebugReady() const
{
	return m_debugRenderableCopies.IsSwapped();
}

bool Scene::IsRender2DReady() const
{
	return m_2dHandler.IsRenderReady();
}

Camera* Scene::GetCurrentCamera() const
{
	return &m_currentCamera.GetComponent<comp::Camera3D>()->camera;
}

void Scene::ReadyForSwap()
{
	m_renderableCopies.ReadyForSwap();
	m_debugRenderableCopies.ReadyForSwap();
	m_renderableAnimCopies.ReadyForSwap();
}

void Scene::SetCurrentCameraEntity(Entity cameraEntity)
{
	if (!cameraEntity.IsNull() && cameraEntity.GetComponent<comp::Camera3D>())
	{
		m_currentCamera = cameraEntity;
	}
	else
	{
		LOG_ERROR("Entity does not have a Camera3D component!");
	}
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
