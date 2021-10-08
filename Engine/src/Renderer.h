#pragma once
#include "BasePass.h"
#include "DepthPass.h"
#include "IRenderPass.h"
#include "PipelineManager.h"


class Renderer
{
private:
	D3D11Core* m_d3d11;
	PipelineManager m_pipelineManager;
	std::vector<IRenderPass*> m_passes;
	
	BasePass m_basePass;	// Forward Rendering.
	DepthPass m_depthPass;	// Forward Plus (1st pass).
	unsigned int m_currentPass = 0;

	
	// Add a pass to the list.
	void AddPass(IRenderPass* pass);

public:
	Renderer();
	virtual ~Renderer() = default;

	void Initialize(Window* pWindow);

	// Clears the screen.
	void ClearFrame();

	// Call this each frame to render all passes:
	//	PreRender(): set pipeline.
	//	Render(): render all objects.
	//	PostRender(): clear pipeline settings.
	void Render(Scene* pScene);

	IRenderPass* GetCurrentPass() const;
};

