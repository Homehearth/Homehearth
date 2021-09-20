#pragma once
#include "EventTypes.h"

typedef std::function<void(entt::registry&, float)>	SystemUpdateFunction;
typedef std::function<void (entt::registry&)>		SystemRenderFunction;

class Scene : public entt::emitter<Scene>
{
private:
	// ecs stuff
	entt::registry m_ecsRegistry;
	
	std::vector<SystemUpdateFunction> m_updateSystems;
	std::vector<SystemRenderFunction> m_renderSystems;

	
public:

	Scene() = default;
	virtual ~Scene() = default;
	Scene(const Scene&) = delete;
	void operator=(const Scene&) = delete;

	// Load scene from file
	void Load(const std::filesystem::path& path) {};


	entt::entity CreateEntity();
	void DestroyEntity(entt::entity entity);

	template<typename T, typename ...Args>
	T& AddComponent(entt::entity entity, const Args& ...args);

	template<typename T>
	T& GetComponent(entt::entity entity);

	template<typename ...T>
	std::tuple<T...> GetComponents(entt::entity entity);

	template<typename T>
	void RemoveComponent(entt::entity entity);

	// Adds a function to be called when the scene is updated
	void AddSystem(const SystemUpdateFunction& updateFunction);

	// Adds a function to be called when the scene is rendered
	void AddRenderSystem(const SystemRenderFunction& renderFunction);

	// Update all systems
	void Update(float dt);

	// Draw all renderSystems
	void Render();

};

template<typename T, typename ...Args>
inline T& Scene::AddComponent(entt::entity entity, const Args & ...args)
{
	return m_ecsRegistry.emplace<T>(entity, args...);
}

template<typename T>
inline T& Scene::GetComponent(entt::entity entity)
{
	return m_ecsRegistry.get<T>(entity);
}

template<typename ...T>
inline std::tuple<T...> Scene::GetComponents(entt::entity entity)
{
	return m_ecsRegistry.get<T...>(entity);
}

template<typename T>
inline void Scene::RemoveComponent(entt::entity entity) 
{
	m_ecsRegistry.remove<T>(entity);
}
