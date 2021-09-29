#pragma once
#include "EventTypes.h"

#include "Components.h"

#include "DoubleBuffer.h"

class Scene : public entt::emitter<Scene>
{
private:
	// Registry handles all ecs data
	entt::registry m_registry;
	
	DoubleBuffer<std::unordered_map<entt::entity, comp::Renderable>> m_renderableCopies;
	dx::ConstantBuffer<sm::Matrix> m_publicBuffer;

	std::mutex m_destroyedEntityMutex;
	void OnRenderableDestroy(entt::registry& reg, entt::entity e);
public:

	Scene();
	virtual ~Scene() = default;
	Scene(const Scene&) = delete;
	void operator=(const Scene&) = delete;

	entt::registry& GetRegistry();

	// Emit update event and update constant buffers
	void Update(float dt);

	// Emit render event and render Renderable components
	void Render();

	bool IsRenderReady() const;

};
