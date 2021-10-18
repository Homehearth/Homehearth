#pragma once
#include "HeadlessScene.h"

class Scene : public HeadlessScene
{
private:
	bool m_IsRenderingColliders;
	DoubleBuffer<std::vector<comp::Renderable>> m_renderableCopies;
	DoubleBuffer<std::vector<comp::RenderableDebug>> m_debugRenderableCopies;
	dx::ConstantBuffer<basic_model_matrix_t> m_publicBuffer;
	dx::ConstantBuffer<collider_hit_t> m_ColliderHitBuffer;
	Camera* m_currentCamera;
	Camera m_defaultCamera;
public:
	Scene();

	// Emit update event and update constant buffers
	virtual void Update(float dt) override;

	// Emit render event and render Renderable components
	void Render();
	void RenderDebug();

	const bool IsRenderReady() const;
	const bool IsRenderDebugReady() const;

	Camera* GetCurrentCamera()const;
	void SetCurrentCamera(Camera* pCamera);

	//ImGui data for disable/enable 
	bool* GetIsRenderingColliders();
	
	DoubleBuffer<std::vector<comp::Renderable>>* GetBuffers();
	DoubleBuffer<std::vector<comp::RenderableDebug>>* GetDebugBuffers();
	void ReadyForSwap();
	
	DoubleBuffer<std::vector<comp::Renderable>>* GetDoubleBuffers();
};
