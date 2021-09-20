#include "EnginePCH.h"
#include "Scene.h"

Scene::Scene() 
{

}

entt::registry& Scene::GetRegistry() {
	return m_registry;
}

void Scene::Update(float dt)
{
	publish<ESceneUpdate>(dt);
}

void Scene::Render() 
{
	publish<ESceneRender>();
}