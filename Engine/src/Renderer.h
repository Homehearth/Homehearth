#pragma once
#include "BasePass.h"
#include "DepthPass.h"
#include "DebugPass.h"
#include "AnimationPass.h"
#include "PipelineManager.h"
#include "DecalPass.h"
#include "ShadowPass.h"
#include "ParticlePass.h"
#include "SkyboxPass.h"
#include "BlurPass.h"
#include "DepthOfFieldPass.h"


class Renderer
{
private:
	D3D11Core* m_d3d11;
	PipelineManager m_pipelineManager;
	std::vector<IRenderPass*> m_passes;

	BasePass		m_basePass;	
	DepthPass		m_depthPass;
	DebugPass		m_debugPass;
	AnimationPass	m_animPass;
	DecalPass		m_decalPass;
	ParticlePass	m_particlePass;
	SkyboxPass		m_skyPass;
	ShadowPass		m_shadowPass;
	DOFPass			m_dofPass;
	unsigned int m_currentPass = 0;

	// Update per frame related resources.
	void UpdatePerFrame(Camera* pCam);
	
	// Add a pass to the list.
	void AddPass(IRenderPass* pass);

public:
	Renderer();
	virtual ~Renderer() = default;

	void Initialize(Window* pWindow);
	void Setup(BasicEngine<Scene>& engine);

	void ClearFrame();

	void Render(Scene* pScene);

	IRenderPass* GetCurrentPass() const;
	//TEMP PLZ REMOVE AFTER WE COME TO AN AGREEMENT ON WHICH DOF EFFECT TO USE
	DOFPass* GetDoFPass();
};

