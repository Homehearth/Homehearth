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
	
	template<class E>
	E* GetElement(const std::string& element_name) const;

	void Insert2DElement(Element2D* element, std::string& name);
	void Insert2DElement(Element2D* element, std::string&& name = "");

	Camera* GetCurrentCamera()const;
	void SetCurrentCameraEntity(Entity cameraEntity);

	//ImGui data for disable/enable 
	bool* GetIsRenderingColliders();
	Lights* GetLights();
	
	DoubleBuffer<std::vector<comp::Renderable>>* GetBuffers();
	DoubleBuffer<std::vector<comp::RenderableDebug>>* GetDebugBuffers();
	void ReadyForSwap();
};

template<class E>
inline E* Scene::GetElement(const std::string& element_name) const
{
	return m_2dHandler.GetElement<E>(element_name);
}
