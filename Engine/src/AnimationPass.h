#pragma once
#include "IRenderPass.h"
#include "ShadowPass.h"

//--------------------------------------
// Forward Rendering, or GeometryPass.
//--------------------------------------
class AnimationPass : public IRenderPass
{
public:
	Skybox* m_skyboxRef = nullptr;

	ShadowPass* m_pShadowPass;

	AnimationPass() = default;
	virtual ~AnimationPass() = default;

	void PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

	void Render(Scene* pScene) override;

	void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
};
