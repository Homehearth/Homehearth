#include "EnginePCH.h"
#include "Scene.h"
#include "BackBuffer.h"

Scene::Scene() 
{
	m_registry.on_construct<comp::Renderable>().connect<ecs::OnRenderableConstruct>();
	m_hasRendered = true;
	publicBuffer.Create(D3D11Core::Get().Device());
}

entt::registry& Scene::GetRegistry() {
	return m_registry;
}

void Scene::Update(float dt)
{
	// Emit event
	publish<ESceneUpdate>(dt);
	
	Backbuffer::GetBuffers()->GetBuffer(0)->clear();
	auto v = m_registry.view<comp::Renderable, comp::Transform>();
	v.each([](comp::Renderable& rend, comp::Transform& transf) {
		comp::Renderable Render;
		Render.mesh = rend.mesh;
		Render.renderForm = transf;
		Backbuffer::GetBuffers()->GetBuffer(0)->push_back(Render);
	});

	if (!Backbuffer::GetBuffers()->IsSwapped())
	{
		Backbuffer::GetBuffers()->SwapBuffers();
	}
}

void Scene::Render() 
{

	std::vector<comp::Renderable>* data = Backbuffer::GetBuffers()->GetBuffer(1);
	comp::Renderable* object = nullptr;

	if (data)
	{
		// Loop through each object.
		for (int i = 0; i < data->size(); i++)
		{
			// Render objects.
			object = &data->at(i);
			if (!object)
				continue;

			publicBuffer.SetData(D3D11Core::Get().DeviceContext(), ecs::GetMatrix(object->renderForm));

			ID3D11Buffer* buffer[1];
			buffer[0] = publicBuffer.GetBuffer();
			D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(0, 1, buffer);

			if (object->mesh)
				object->mesh->Render();

		}
	}
	// Emit event
	publish<ESceneRender>();
}