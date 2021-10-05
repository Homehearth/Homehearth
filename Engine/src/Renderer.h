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
	
	DepthPass m_depthPass;	// Depth, pre Z-pass.
	BasePass m_basePass;	// Forward, "GeometryPass".
	
	// Add a pass to the list.
	void AddPass(IRenderPass* pass);

	std::string GetInfoAboutPasses();

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

};

