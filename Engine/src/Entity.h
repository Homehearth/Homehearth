#pragma once

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
	
	template<typename T>
	T* AddComponent();

	template<typename T>
	void RemoveComponent();

	void Destroy();

	bool IsNull() const;

	operator entt::entity()const
	{
		return m_entity;
	}

	bool operator >(const Entity& other) const
	{
		return m_entity > other.m_entity;
	}
};

template<typename T>
inline T* Entity::GetComponent() const
{
	if (this->IsNull())
	{
		throw std::runtime_error("Entity was a null entity");
	}
	return m_pRegistry->try_get<T>(m_entity);
}

template<typename T>
inline T* Entity::AddComponent()
{
	if (this->IsNull())
	{
		throw std::runtime_error("Entity was a null entity");
	}	
	return &m_pRegistry->emplace_or_replace<T>(m_entity);
}

template<typename T>
inline void Entity::RemoveComponent()
{
	if (this->IsNull())
	{
		throw std::runtime_error("Entity was a null entity");
	}	
	m_pRegistry->erase<T>(m_entity);
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