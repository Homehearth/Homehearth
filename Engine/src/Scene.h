#pragma once
#include "HeadlessScene.h"
#include "Lights.h"

struct RenderableAnimation
{
	comp::Renderable renderable;
	comp::Animator animator;
};

class Scene : public BasicScene<Scene>
{
private:
	bool m_IsRenderingColliders;
	DoubleBuffer<std::vector<comp::Renderable>> m_renderableCopies;
	DoubleBuffer<std::vector<comp::RenderableDebug>> m_debugRenderableCopies;
	DoubleBuffer<std::vector<RenderableAnimation>> m_renderableAnimCopies;
	//DoubleBuffer<std::vector<comp::RenderableAnimation>> m_renderableAnimCopies;


	dx::ConstantBuffer<basic_model_matrix_t> m_publicBuffer;
	dx::ConstantBuffer<collider_hit_t> m_ColliderHitBuffer;
	Entity m_currentCamera;
	Entity m_defaultCamera;

	Lights m_lights;

public:
	Scene();

	// Emit update event and update constant buffers
	virtual void Update(float dt) override;

	// Emit render event and render Renderable components
	void Render();
	void RenderDebug();
	void RenderAnimation();

	//Checks all of the following underneath
	const bool IsReadyToRenderAll() const;
	const bool IsRenderReady() const;
	const bool IsRenderDebugReady() const;
	const bool IsAnimRenderReady() const;

	Camera* GetCurrentCamera()const;
	void SetCurrentCameraEntity(Entity cameraEntity);

	//ImGui data for disable/enable 
	bool* GetIsRenderingColliders();
	Lights* GetLights();
	
	DoubleBuffer<std::vector<comp::Renderable>>*			GetBuffers();
	DoubleBuffer<std::vector<comp::RenderableDebug>>*		GetDebugBuffers();
	DoubleBuffer<std::vector<RenderableAnimation>>*			GetAnimationBuffers();
	void ReadyForSwap();
	
	DoubleBuffer<std::vector<comp::Renderable>>* GetDoubleBuffers();
};
