#pragma once
#include "ShadowPass.h"
#include "Skybox.h"

class OpaquePass : public IRenderPass
{
public:
	ShadowPass* m_shadowPassRef = nullptr;
	Skybox* m_skyBoxRef = nullptr;

	void PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext) override;
	void Render(Scene* pScene) override;
	void PostRender(ID3D11DeviceContext* pDeviceContext) override;
};

