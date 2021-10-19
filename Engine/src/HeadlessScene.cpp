#include "EnginePCH.h"
#include "HeadlessScene.h"

HeadlessScene::HeadlessScene()
{

}

Entity HeadlessScene::CreateEntity()
{
	return Entity(m_registry);
}

entt::registry* HeadlessScene::GetRegistry()
{
	return &this->m_registry;
}

void HeadlessScene::Update(float dt)
{
	PROFILE_FUNCTION();
	// Emit event
	publish<ESceneUpdate>(dt);
}

