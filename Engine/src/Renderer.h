#pragma once
#include "BasePass.h"
#include "DepthPass.h"
#include "DebugPass.h"
#include "AnimationPass.h"
#include "CullingPass.h"
#include "FrustumPass.h"
#include "OpaqPass.h"
#include "PipelineManager.h"
#include "TransPass.h"


class Renderer
{
private:
	D3D11Core* m_d3d11;
	PipelineManager m_pipelineManager;
	std::vector<IRenderPass*> m_passes;

	BasePass		m_basePass;

	DepthPass		m_depthPass;
	FrustumPass		m_frustumPass;
	CullingPass		m_cullingPass;
	OpaqPass		m_opaqPass;
	TransPass		m_transPass;
	AnimationPass	m_animPass;
	DebugPass		m_debugPass;


	unsigned int m_currentPass = 0;

	// Update per frame related resources.
	void UpdatePerFrame(Camera* pCam);
	
	// Add a pass to the list.
	void AddPass(IRenderPass* pass);

public:
	Renderer();
	virtual ~Renderer() = default;

	void Initialize(Window* pWindow);

	void ClearFrame();

	void Render(Scene* pScene);

	void OnWindowResize();

	IRenderPass* GetCurrentPass() const;
};

