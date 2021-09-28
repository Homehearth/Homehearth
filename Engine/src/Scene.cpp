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
	PROFILE_FUNCTION();
	// Emit event
	publish<ESceneUpdate>(dt);
	
	// only copy if the last frame has been rendered
	if (m_hasRendered)
	{
		PROFILE_SCOPE("Copy Transforms");
		m_registry.view<comp::Transform>().each([&](entt::entity e, comp::Transform& t) 
			{
				m_transformCopies[e] = t;
			});
		m_hasRendered = false;
	Backbuffer::GetBuffers()->GetBuffer(0)->clear();
	auto v = m_registry.group<comp::Renderable, comp::Transform>();
	v.each([](const comp::Renderable& rend, const comp::Transform& transf) {
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
	PROFILE_FUNCTION();
	while (m_hasRendered); // makes sure render thread is not faster than update thread

	std::vector<comp::Renderable>* data = Backbuffer::GetBuffers()->GetBuffer(1);
	comp::Renderable* object = nullptr;

	// System that renders Renderable component
	auto view = m_registry.view<comp::Renderable>();
	view.each([&](entt::entity e, comp::Renderable& renderable)
		{
			PROFILE_SCOPE("Render Renderable");
			if (m_transformCopies.find(e) != m_transformCopies.end())
			{
				comp::Transform transform = m_transformCopies.at(e);
				sm::Matrix m = ecs::GetMatrix(transform);
				renderable.constantBuffer.SetData(D3D11Core::Get().DeviceContext(), m);
			}
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