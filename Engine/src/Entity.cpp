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

void Entity::UpdateNetwork()
{
	if (GetComponent<comp::Network>())
	{
		m_pRegistry->patch<comp::Network>(m_entity);
	}
}

bool Entity::Destroy() 
{
	if (IsNull())
	{
		return false;
	}

	m_pRegistry->destroy(m_entity);
	return true;
}

bool Entity::IsNull() const
{
	return m_entity == entt::null || !m_pRegistry->valid(m_entity);
}

tag_bits Entity::GetTags() const
{
	tag_bits tagBits = 0;

	using namespace entt::literals;
	for (auto type : entt::resolve())
	{
		if (type.func("has"_hs).invoke({}, *this).cast<bool>())
		{
			
			auto instance = type.func("get"_hs).invoke({}, *this);
			comp::ITag* tag = instance.try_cast<comp::ITag>();
			if (!tag)
				continue; // this is not a tag type
			tagBits |= tag->id;
		}
	}
	return tagBits;
}


