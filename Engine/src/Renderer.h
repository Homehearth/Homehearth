#pragma once
#include "BasePass.h"
#include "DepthPass.h"
#include "IRenderPass.h"
#include "PipelineManager.h"


class Renderer
{
private:
	D3D11Core* m_d3d11;
	Camera* m_camera;
	PipelineManager m_pipelineManager;
	std::vector<IRenderPass*> m_passes;
	
	BasePass m_basePass;	
	DepthPass m_depthPass;

	// Update per frame related resources.
	void UpdatePerFrame();
	
	// Add a pass to the list.
	void AddPass(IRenderPass* pass);

public:
	Renderer();
	virtual ~Renderer() = default;

	void Initialize(Window* pWindow);

	void ClearFrame();

	void Render(Scene* pScene);

};

