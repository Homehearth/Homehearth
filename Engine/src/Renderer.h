#pragma once
#include "BasePass.h"
#include "IRenderPass.h"
#include "PipelineManager.h"


class Renderer
{
private:
	D3D11Core* m_d3d11;
	PipelineManager m_pipelineManager;
	std::vector<IRenderPass*> m_passes;
	
	BasePass m_basePass;	// Forward Rendering.

public:
	Renderer();
	virtual ~Renderer() = default;

	void Initialize(Window* pWindow);
	
	void ClearFrame();
	
	void Render();

	// Add pass that are created outside the Renderer.
	void AddPass(IRenderPass* pass);

	// temporary tester
	void SetPipelineState();
};

