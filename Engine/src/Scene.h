#pragma once
#include "EventTypes.h"

#include <entt.hpp>
#include <vector>

typedef std::function<void(entt::registry&, float)>	SystemUpdateFunction;
typedef std::function<void (entt::registry&)>	SystemRenderFunction;

class Scene
{
private:
	static entt::dispatcher m_staticEventDispatcher;
	// ecs stuff
	entt::registry m_ecsRegistry;
	
	std::vector<SystemUpdateFunction> m_updateSystems;
	std::vector<SystemRenderFunction> m_renderSystems;

	
public:

	static entt::dispatcher& GetEventDispatcher() { return m_staticEventDispatcher; }


	Scene() = default;
	virtual ~Scene() = default;
	Scene(const Scene&) = delete;
	void operator=(const Scene&) = delete;

	// Load scene from file
	void Load(const std::filesystem::path& path) {};

	/*
	entt::entity CreateEntity();
	void DestroyEntity(entt::entity entity);

	template<typename T, typename ...Args>
	T& AddComponent(entt::entity entity, const Args& ...);

	template<typename T>
	T& GetComponent(entt::entity entity);

	template<typename ...T>
	std::tuple<T...> GetComponents(entt::entity entity);

	template<typename T>
	void RemoveComponent(entt::entity entity);

	*/


	// Adds a function to be called when the scene is updated
	void AddSystem(const SystemUpdateFunction& updateFunction);

	// Adds a function to be called when the scene is rendered
	void AddRenderSystem(const SystemRenderFunction& renderFunction);

	// Update all systems
	void Update(float dt);

	// Draw all renderSystems
	void Render();

};
