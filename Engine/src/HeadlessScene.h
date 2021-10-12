#pragma once
#include "EventTypes.h"
#include "Components.h"
#include "DoubleBuffer.h"
#include "Entity.h"
#include "Camera.h"

class HeadlessScene : public entt::emitter<HeadlessScene>
{
protected:

	// Registry handles all ecs data
	entt::registry m_registry;

public:

	HeadlessScene();
	virtual ~HeadlessScene() = default;
	HeadlessScene(const HeadlessScene&) = delete;
	void operator=(const HeadlessScene&) = delete;

	Entity CreateEntity();

	template<typename ...T>
	void ForEachComponent(std::function<void(T&...)> func);

	template<typename ...T>
	void ForEachComponent(std::function<void(Entity, T&...)> func);


	// Emit update event and update constant buffers
	virtual void Update(float dt);

};



template<typename ...T>
inline void HeadlessScene::ForEachComponent(std::function<void(T&...)> func) {
	m_registry.view<T...>().each(func);
}

template<typename ...T>
inline void HeadlessScene::ForEachComponent(std::function<void(Entity, T&...)> func) {
	m_registry.view<T...>().each([&](entt::entity e, T&... comp)
		{
			Entity entity(m_registry, e);
			func(entity, comp...);
		});
}