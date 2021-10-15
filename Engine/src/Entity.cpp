#include "EnginePCH.h"

Entity::Entity(entt::registry& registry)
{
	m_pRegistry = &registry;
	m_entity = m_pRegistry->create();
}

Entity::Entity(entt::registry& registry, entt::entity id)
{
	m_pRegistry = &registry;
	m_entity = id;
}

Entity::Entity()
{
	m_pRegistry = nullptr;
	m_entity = entt::null;
}

void Entity::Destroy() 
{
	m_pRegistry->destroy(m_entity);
}

bool Entity::IsNull() const
{
	return m_entity == entt::null;
}


