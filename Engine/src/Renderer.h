#pragma once
#include "BasePass.h"
#include "DepthPass.h"
#include "DebugPass.h"
#include "AnimationPass.h"
#include "TextureEffectPass.h"
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
	TextureEffectPass m_textureEffectPass; // Water effect pass.

	DebugPass		m_debugPass;
	AnimationPass	m_animPass;
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

	IRenderPass* GetCurrentPass() const;
};

