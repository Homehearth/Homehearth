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
	bool m_IsRenderingColliders;
	
	DoubleBuffer<std::vector<comp::Renderable>> m_renderableCopies;
	dx::ConstantBuffer<basic_model_matrix_t> m_publicBuffer;
	//Debug renderables
	DoubleBuffer<std::vector<comp::RenderableDebug>> m_debugRenderableCopies;
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

	template<typename... Ts, typename F>
	void ForEachOrComponent(F&& f);
	
	entt::basic_registry<entt::entity>* GetRegistry();

	// Emit update event and update constant buffers
	void Update(float dt);

	// Emit render event and render Renderable components
	void Render();
	void RenderDebug();

	const bool IsRenderReady() const;
	const bool IsRenderDebugReady() const;
	
	Camera* GetCamera();
	std::shared_ptr<Camera> m_currentCamera;

	//ImGui data for disable/enable 
	bool* GetIsRenderingColliders();
	void InitRenderableColliders();
	
	DoubleBuffer<std::vector<comp::Renderable>>* GetBuffers();
	DoubleBuffer<std::vector<comp::RenderableDebug>>* GetDebugBuffers();
	void ReadyForSwap();
	
	DoubleBuffer<std::vector<comp::Renderable>>* GetDoubleBuffers();
};

template<typename ...Ts, typename F>
inline void Scene::ForEachOrComponent(F &&f)
{
	(m_registry.view<Ts>().each(f), ...);
}

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