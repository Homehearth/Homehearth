#pragma once

//--------------------------------------
// When this pass is active, it means that
// we need to update the frustum grid and
// related resources. 
//--------------------------------------
class FrustumPass : public IRenderPass
{
public:
	// Inherited via IRenderPass
	void PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext) override;
	void Render(Scene* pScene) override;
	void PostRender(ID3D11DeviceContext* pDeviceContext) override;
};

