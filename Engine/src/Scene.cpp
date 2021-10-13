#include "EnginePCH.h"
#include "Scene.h"

Scene::Scene()
	: m_currentCamera(nullptr)
{	
	m_publicBuffer.Create(D3D11Core::Get().Device());
}

void Scene::Update(float dt)
{
	PROFILE_FUNCTION();

	// Emit event
	HeadlessScene::Update(dt);

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

	// System that renders Renderable component

	ID3D11Buffer* buffers[1] =
	{
		m_publicBuffer.GetBuffer()
	};

	D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(0, 1, buffers);
	for (const auto& it : m_renderableCopies[1])
	{
		m_publicBuffer.SetData(D3D11Core::Get().DeviceContext(), it.data);
		if (it.model)
			it.model->Render();	
	}
	
	// Emit event
	publish<ESceneRender>();

	m_renderableCopies.ReadyForSwap();
}

const bool Scene::IsRenderReady() const
{
	return m_renderableCopies.IsSwapped();
}

Camera* Scene::GetCurrentCamera() const
{
	return m_currentCamera;
}

void Scene::SetCurrentCamera(Camera* pCamera)
{
	m_currentCamera = pCamera;
}

DoubleBuffer<std::vector<comp::Renderable>>* Scene::GetBuffers()
{
	return &m_renderableCopies;
}
