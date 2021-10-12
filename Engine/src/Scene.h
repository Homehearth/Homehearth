#pragma once
#include "HeadlessScene.h"

class Scene : public HeadlessScene
{
private:

	DoubleBuffer<std::vector<comp::Renderable>> m_renderableCopies;
	dx::ConstantBuffer<basic_model_matrix_t> m_publicBuffer;

public:

	Scene();


	// Emit update event and update constant buffers
	virtual void Update(float dt) override;

	// Emit render event and render Renderable components
	void Render();

	const bool IsRenderReady() const;

	Camera* GetCamera();
	std::shared_ptr<Camera> m_currentCamera;

	DoubleBuffer<std::vector<comp::Renderable>>* GetBuffers();
};
