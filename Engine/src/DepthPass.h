#pragma once
#include "IRenderPass.h"

//--------------------------------------
// Write the depth values of the scene
// from the camera's perspective into
// a depth buffer.
//--------------------------------------
class DepthPass : public IRenderPass
{
public:
	DepthPass() = default;
	virtual ~DepthPass() = default;

	void PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

	void Render(Scene* pScene) override;

	void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

};

