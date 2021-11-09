#pragma once
#include "HeadlessScene.h"
#include "Lights.h"
#include "Handler2D.h"

class Scene : public BasicScene<Scene>
{
private:
	bool m_IsRenderingColliders;
	bool m_updateAnimation;
	DoubleBuffer<std::vector<comp::Renderable>> m_renderableCopies;
	DoubleBuffer<std::vector<comp::RenderableDebug>> m_debugRenderableCopies;
	DoubleBuffer<std::vector<std::pair<comp::Renderable,comp::Animator>>> m_renderableAnimCopies;

	dx::ConstantBuffer<basic_model_matrix_t> m_publicBuffer;
	dx::ConstantBuffer<collider_hit_t> m_ColliderHitBuffer;
	dx::ConstantBuffer<camera_Matrix_t> m_publicDecalBuffer;
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

	/*
		Updates both 3d and 2d.
	*/
	virtual void Update(float dt) override;

	/*
		Updates only the 2d scene.
	*/
	void Update2D();

	// Emit render event and render Renderable components
	void Render();
	void RenderDebug();
	void RenderAnimation();
	void Render2D();

	/*
		Decalpass Functions.
	*/
	void RenderDecals();

	bool IsRenderReady() const;

	void Add2DCollection(Collection2D* collection, std::string& name);
	void Add2DCollection(Collection2D* collection, const char* name);
	Collection2D* GetCollection(const std::string& name);

	Camera* GetCurrentCamera()const;
	void SetCurrentCameraEntity(Entity cameraEntity);

	//ImGui data for disable/enable 
	bool* GetIsRenderingColliders();
	Lights* GetLights();
	
	DoubleBuffer<std::vector<comp::Renderable>>*		GetBuffers();
	DoubleBuffer<std::vector<comp::RenderableDebug>>*	GetDebugBuffers();
	void ReadyForSwap();
};
