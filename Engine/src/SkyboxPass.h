#pragma once
#include "IRenderPass.h"

class SkyboxPass : public IRenderPass
{
public:
	SkyboxPass() = default;
	virtual ~SkyboxPass() = default;

	// Inherited via IRenderPass
	virtual void PreRender(Camera* pCam = nullptr, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
	virtual void Render(Scene* pScene) override;
	virtual void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
};