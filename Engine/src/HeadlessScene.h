#pragma once
#include "EventTypes.h"
#include "DoubleBuffer.h"
#include "Entity.h"
#include "Camera.h"

template<typename Inheriter>
class BasicScene : public entt::emitter<Inheriter>
{
protected:

	// Registry handles all ecs data
	entt::registry m_registry;

public:

	BasicScene();
	virtual ~BasicScene() = default;
	BasicScene(const BasicScene&) = delete;
	void operator=(const BasicScene&) = delete;

	Entity CreateEntity();
	entt::registry* GetRegistry();

	template<typename ...Ts, typename F>
	void ForEachOrComponent(F&& f);

	template<typename ...T, typename F>
	void ForEachComponent(F func);

	// Emit update event and update constant buffers
	virtual void Update(float dt);

	void Clear();

};

template<typename Inheriter>
BasicScene<Inheriter>::BasicScene()
{

}

template<typename Inheriter>
Entity BasicScene<Inheriter>::CreateEntity()
{
	return Entity(m_registry);
}

template<typename Inheriter>
entt::registry* BasicScene<Inheriter>::GetRegistry()
{
	return &this->m_registry;
}

template<typename Inheriter>
void BasicScene<Inheriter>::Update(float dt)
{
	//PROFILE_FUNCTION();

	// Emit event
	publish<ESceneUpdate>(dt);
}

template<typename Inheriter>
void BasicScene<Inheriter>::Clear()
{
	m_registry.clear();
}


template<typename Inheriter>
template<typename ...Ts, typename F>
void BasicScene<Inheriter>::ForEachOrComponent(F&& f)
{
	(m_registry.view<Ts>().each(f), ...);
}

template<typename Inheriter>
template<typename ...T, typename F>
void BasicScene<Inheriter>::ForEachComponent(F func) {
	static_assert(
		std::is_assignable_v<std::function<void(Entity, T&...)>, F> ||
		std::is_assignable_v<std::function<void(T&...)>, F> && 
		"Not a valid function signature");

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
	
}

class HeadlessScene : public BasicScene<HeadlessScene>
{

};