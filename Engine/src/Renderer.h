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
#include "DecalPass.h"
#include "ShadowPass.h"
#include "ParticlePass.h"
#include "SkyboxPass.h"
#include "BlurPass.h"
#include "DepthOfFieldPass.h"
#include "HeadlessEngine.h"

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

	DecalPass		m_decalPass;
	ParticlePass	m_particlePass;
	SkyboxPass		m_skyPass;
	ShadowPass		m_shadowPass;
	DOFPass			m_dofPass;
	unsigned int m_currentPass = 0;
	bool m_isForwardPlusInitialized;


	// Update per frame related resources.
	void UpdatePerFrame(Camera* pCam);
	
	// Add a pass to the list.
	void AddPass(IRenderPass* pass);

	// Initialize ForwardPlus. Called on create and on window resize.
	void InitilializeForwardPlus(Camera* camera);
	bool CreateLightGridRWB();
	bool CreateLightIndexListRWB(const uint32_t& COUNT);
	bool CreateLightIndexCounterRWB();
public:
	Renderer();
	virtual ~Renderer() = default;

	void Initialize(Window* pWindow);
	void Setup(BasicEngine<Scene>& engine);

	void ClearFrame();

	void Render(Scene* pScene);

	void OnWindowResize();

	IRenderPass* GetCurrentPass() const;
	//TEMP PLZ REMOVE AFTER WE COME TO AN AGREEMENT ON WHICH DOF EFFECT TO USE
	DOFPass* GetDoFPass();
};

