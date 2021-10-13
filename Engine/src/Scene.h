#pragma once
#include "HeadlessScene.h"

class Scene : public HeadlessScene
{
private:

	DoubleBuffer<std::vector<comp::Renderable>> m_renderableCopies;
	dx::ConstantBuffer<basic_model_matrix_t> m_publicBuffer;
	Camera* m_currentCamera;
public:
	Scene();

	// Emit update event and update constant buffers
	virtual void Update(float dt) override;

	// Emit render event and render Renderable components
	void Render();

	const bool IsRenderReady() const;

	Camera* GetCurrentCamera()const;
	void SetCurrentCamera(Camera* pCamera);

	DoubleBuffer<std::vector<comp::Renderable>>* GetBuffers();
};
