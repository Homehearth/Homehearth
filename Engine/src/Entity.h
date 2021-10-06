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
	Entity();
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
	if (m_entity == entt::null)
	{
		throw std::runtime_error("Entity was a null entity");
	}
	return m_registry->try_get<T>(m_entity);
}

template<typename T>
inline T* Entity::AddComponent()
{
	if (m_entity == entt::null)
	{
		throw std::runtime_error("Entity was a null entity");
	}	
	return &m_registry->emplace_or_replace<T>(m_entity);
}

template<typename T>
inline void Entity::RemoveComponent()
{
	if (m_entity == entt::null)
	{
		throw std::runtime_error("Entity was a null entity");
	}	
	m_registry->erase<T>(m_entity);
}
