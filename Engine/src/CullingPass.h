#pragma once
#include "IRenderPass.h"
class CullingPass : public IRenderPass
{
public:
	CullingPass() = default;
	virtual ~CullingPass() = default;

	void PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext) override;
	void Render(Scene* pScene) override;
	void PostRender(ID3D11DeviceContext* pDeviceContext) override;
};

