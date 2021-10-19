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
	entt::registry* GetRegistry();

	template<typename ...Ts, typename F>
	void ForEachOrComponent(F&& f);

	template<typename ...T, typename F>
	void ForEachComponent(F func);

	// Emit update event and update constant buffers
	virtual void Update(float dt);

};


template<typename ...Ts, typename F>
void HeadlessScene::ForEachOrComponent(F&& f)
{
	(m_registry.view<Ts>().each(f), ...);
}

template<typename ...T, typename F>
void HeadlessScene::ForEachComponent(F func) {
	if constexpr (std::is_assignable_v<std::function<void(Entity, T&...)>, F>) 
	{
		m_registry.view<T...>().each([&](entt::entity e, T&... comp)
			{
				Entity entity(m_registry, e);
				func(entity, comp...);
			});
	}
	else if constexpr (std::is_assignable_v<std::function<void(T&...)>, F>)
	{
		m_registry.view<T...>().each(func);
	}
	else
	{
		throw std::runtime_error("No valid function argument");
	}

}
