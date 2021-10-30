#pragma once
#include "HeadlessScene.h"
#include "Lights.h"
#include "Handler2D.h"

class Scene : public BasicScene<Scene>
{
private:
	bool m_IsRenderingColliders;
	DoubleBuffer<std::vector<comp::Renderable>> m_renderableCopies;
	DoubleBuffer<std::vector<comp::RenderableDebug>> m_debugRenderableCopies;
	dx::ConstantBuffer<basic_model_matrix_t> m_publicBuffer;
	dx::ConstantBuffer<collider_hit_t> m_ColliderHitBuffer;
	Entity m_currentCamera;
	Entity m_defaultCamera;
	Handler2D m_2dHandler;

	Lights m_lights;

	bool IsRender3DReady() const;
	bool IsRenderDebugReady() const;
	bool IsRender2DReady() const;

public:
	Scene();

	// Emit update event and update constant buffers
	virtual void Update(float dt) override;

	// Emit render event and render Renderable components
	void Render();
	void RenderDebug();
	void Render2D();

	bool IsRenderReady() const;

	void Add2DCollection(Collection2D* collection, std::string& name);
	void Add2DCollection(Collection2D* collection, const char* name);

	Camera* GetCurrentCamera()const;
	void SetCurrentCameraEntity(Entity cameraEntity);

	//ImGui data for disable/enable 
	bool* GetIsRenderingColliders();
	Lights* GetLights();
	
	DoubleBuffer<std::vector<comp::Renderable>>* GetBuffers();
	DoubleBuffer<std::vector<comp::RenderableDebug>>* GetDebugBuffers();
	void ReadyForSwap();
};
