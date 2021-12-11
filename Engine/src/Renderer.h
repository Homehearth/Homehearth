#pragma once
#include "BasePass.h"
#include "DepthPass.h"
#include "DebugPass.h"
#include "AnimationPass.h"
#include "TextureEffectPass.h"
#include "IRenderPass.h"
#include "PipelineManager.h"
#include "DecalPass.h"
#include "ShadowPass.h"
#include "ParticlePass.h"
#include "SkyboxPass.h"
#include "BlurPass.h"
#include "CullingPass.h"
#include "DepthOfFieldPass.h"
#include "FrustumPass.h"
#include "HeadlessEngine.h"
#include "OpaquePass.h"
#include "TransparentPass.h"
#include "WaterEffectPass.h"
#include "BloomPass.h"

class Renderer
{
private:
	D3D11Core* m_d3d11;
	PipelineManager m_pipelineManager;
	std::vector<IRenderPass*> m_passes;
	
	BasePass			m_basePass;	
	DepthPass			m_depthPass;	
	TextureEffectPass	m_textureEffectPass; 
	WaterEffectPass		m_waterEffectPass; 
	FrustumPass			m_frustumPass;
	CullingPass			m_cullingPass;
	OpaquePass			m_opaquePass;
	TransparentPass		m_transparentPass;
	DebugPass			m_debugPass;
	AnimationPass		m_animPass;
	DecalPass			m_decalPass;
	ParticlePass		m_particlePass;
	SkyboxPass			m_skyPass;
	ShadowPass			m_shadowPass;
	DOFPass				m_dofPass;
	BloomPass			m_bloomPass;
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
	bool CreateHeatMapRWB();

public:
	Renderer();
	virtual ~Renderer() = default;

	void Initialize(Window* pWindow);
	void Setup(BasicEngine<Scene>& engine);

	void ClearFrame();

	void Render(Scene* pScene);

	IRenderPass* GetCurrentPass() const;

	DOFPass* GetDoFPass();
	BloomPass* GetBloomPass();

	ShadowPass* GetShadowPass();

	void SetShadowMapSize(uint32_t size);
	uint32_t GetShadowMapSize() const;

	void ImGuiShowTextures();
};

