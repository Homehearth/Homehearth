#pragma once
#include "IRenderPass.h"

// todo: OnWindowResize()
class DepthPass : public IRenderPass
{
public:
	DepthPass() = default;
	virtual ~DepthPass() = default;

	// Inherited via IRenderPass
	void PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext) override;
	void Render(Scene* pScene) override;
	void PostRender(ID3D11DeviceContext* pDeviceContext) override;

};
