#pragma once
#include "EventTypes.h"
#include "Components.h"
#include "DoubleBuffer.h"
#include "Entity.h"
#include "Camera.h"

class Scene : public entt::emitter<Scene>
{
private:

	// Registry handles all ecs data
	entt::registry m_registry;

	DoubleBuffer<std::vector<comp::Renderable>> m_renderableCopies;
	dx::ConstantBuffer<basic_model_matrix_t> m_publicBuffer;

public:

	Scene();
	virtual ~Scene() = default;
	Scene(const Scene&) = delete;
	void operator=(const Scene&) = delete;

	Entity CreateEntity();

	template<typename ...T>
	void ForEachComponent(std::function<void(T&...)> func);
	
	template<typename ...T>
	void ForEachComponent(std::function<void(Entity, T&...)> func); 
	
	void ReadyForSwap();

	// Emit update event and update constant buffers
	void Update(float dt);

	// Emit render event and render Renderable components
	void Render();

	const bool IsRenderReady() const;

	Camera* GetCamera();
	std::shared_ptr<Camera> m_currentCamera;

	DoubleBuffer<std::vector<comp::Renderable>>* GetBuffers();
};



template<typename ...T>
inline void Scene::ForEachComponent(std::function<void(T&...)> func) {
	m_registry.view<T...>().each(func);
}

template<typename ...T>
inline void Scene::ForEachComponent(std::function<void(Entity, T&...)> func) {
	m_registry.view<T...>().each([&](entt::entity e, T&... comp)
		{
			Entity entity(m_registry, e);
			func(entity, comp...);
		});
}