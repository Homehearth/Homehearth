#pragma once

class Entity
{
private:
	entt::registry* m_registry;
	entt::entity m_entity;
	
	friend class Scene;
	Entity(entt::registry& registry);
	Entity(entt::registry& registry, entt::entity id);
public:
	Entity() = delete;
	virtual ~Entity() = default;

	template<typename T>
	T* GetComponent() const;
	
	template<typename T>
	T* AddComponent();

	template<typename T>
	void RemoveComponent();

	void Destroy();

	operator entt::entity()
	{
		return m_entity;
	}
};

template<typename T>
inline T* Entity::GetComponent() const
{
	return m_registry->try_get<T>(m_entity);
}

template<typename T>
inline T* Entity::AddComponent()
{
	return &m_registry->emplace_or_replace<T>(m_entity);
}


template<typename T>
inline void Entity::RemoveComponent()
{
	m_registry->erase<T>(m_entity);
}
