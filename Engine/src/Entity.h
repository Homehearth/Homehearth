#pragma once
#include "Tags.h"

class Entity
{
private:
	entt::registry* m_pRegistry;
	entt::entity m_entity;
	

public:
	Entity(entt::registry& registry);
	Entity(entt::registry& registry, entt::entity id);
	Entity();
	virtual ~Entity() = default;

	template<typename T>
	T* GetComponent() const;
	
	// Only use this if 100% sure this entity has component T
	template<typename T>
	T& GetComponentRef() const;

	template<typename T>
	bool HasComponent() const;

	template<typename T, typename ...Args>
	T* AddComponent(Args&& ...);

	template<typename T>
	void RemoveComponent();

	void UpdateNetwork();
	
	bool Destroy();

	bool IsNull() const;

	tag_bits GetTags() const;

	operator entt::entity()const
	{
		return m_entity;
	}

	operator uint32_t()
	{
		return (uint32_t)m_entity;
	}

	bool operator > (const Entity& other) const
	{
		return m_entity > other.m_entity;
	}

	bool operator == (const Entity& other) const
	{
		return m_entity == other.m_entity && m_pRegistry == other.m_pRegistry;
	}
	
	bool operator != (const Entity& other) const
	{
		return !(m_entity == other.m_entity && m_pRegistry == other.m_pRegistry);
	}

};

template<typename T>
inline T* Entity::GetComponent() const
{
	if (this->IsNull())
	{
		return nullptr;
	}
	return m_pRegistry->try_get<T>(m_entity);
}

template<typename T>
inline T& Entity::GetComponentRef() const
{
	if (this->IsNull())
	{
		throw std::runtime_error("Entity was a null entity");
	}
	return m_pRegistry->get<T>(m_entity);
}

template<typename T>
inline bool Entity::HasComponent() const
{
	return GetComponent<T>() != nullptr;
}

template<typename T, typename ...Args>
inline T* Entity::AddComponent(Args&& ... args)
{
	if (this->IsNull())
	{
		throw std::runtime_error("Entity was a null entity");
	}	
	return &m_pRegistry->emplace_or_replace<T>(m_entity, args...);
}

template<typename T>
inline void Entity::RemoveComponent()
{
	if (this->IsNull())
	{
		throw std::runtime_error("Entity was a null entity");
	}	
	m_pRegistry->remove<T>(m_entity);
}


namespace std
{
	template <> struct hash<Entity>
	{
		size_t operator()(const Entity& other)const
		{
			return std::hash<entt::entity>()((entt::entity)other);
		}
	};
}