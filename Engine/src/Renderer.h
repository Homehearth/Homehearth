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
	Camera* m_camera;
	
	DepthPass m_depthPass;	
	BasePass m_basePass;


	// Add a pass to the list.
	void AddPass(IRenderPass* pass);

	// Updates Camera & Scene-ref once per frame.
	void UpdatePerFrame(Scene* pScene);

	// Debugging info.
	std::string GetInfoAboutPasses();

public:
	Renderer();
	virtual ~Renderer() = default;

	// Initialize Renderer.
	void Initialize(Window* pWindow, Camera* pCamera);

	// Clears the screen.
	void ClearFrame();

	// Call this each frame to render all passes:
	//	PreRender(): set pipeline.
	//	Render(): render all objects.
	//	PostRender(): clear pipeline settings.
	void Render(Scene* pScene);

};


