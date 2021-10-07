#pragma once
#include "EventTypes.h"

#include "Components.h"

#include "DoubleBuffer.h"
#include "Camera.h"

class Scene : public entt::emitter<Scene>
{
private:
	// Registry handles all ecs data
	entt::registry m_registry;
	
	DoubleBuffer<std::vector<comp::Renderable>> m_renderableCopies;
	dx::ConstantBuffer<basic_model_matrix_t> m_publicBuffer;


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

	const bool IsRenderReady() const;

	Camera* GetCamera();
	std::shared_ptr<Camera> m_currentCamera;
	Camera m_gameCamera;
	Camera m_debugCamera;
	sm::Vector3 m_oldGameCameraPosition;
	sm::Vector3 m_oldDebugCameraPosition;

	DoubleBuffer<std::vector<comp::Renderable>>* GetBuffers();
};
