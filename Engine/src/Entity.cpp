#include "EnginePCH.h"

Entity::Entity(entt::registry& registry)
{
	m_registry = &registry;
	m_entity = m_registry->create();
}

Entity::Entity(entt::registry& registry, entt::entity id)
{
	m_registry = &registry;
	m_entity = id;
}

Entity::Entity()
{
	m_registry = nullptr;
	m_entity = entt::null;
}

void Entity::Destroy() 
{
	m_registry->destroy(m_entity);
}


