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

	// Clears the screen.
	void ClearFrame();

	// Call this each frame to all passes:
	//	PreRender(): set pipeline.
	//	Render(): render all objects.
	//	PostRender(): clear pipeline settings.
	void Render();

	// Add a pass to the list. Useful if a pass is created outside the Renderer class.
	void AddPass(IRenderPass* pass);

	// A temporary function used for testing (Forward Rendering).
	void SetPipelineState();
};

