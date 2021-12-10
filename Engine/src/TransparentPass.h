#pragma once
#include "IRenderPass.h"
class TransparentPass : public IRenderPass
{
public:
	void PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext) override;
	void Render(Scene* pScene) override;
	void PostRender(ID3D11DeviceContext* pDeviceContext) override;
};

