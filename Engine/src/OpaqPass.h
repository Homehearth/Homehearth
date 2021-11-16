#pragma once
class OpaqPass : public IRenderPass
{
public:

	// Inherited via IRenderPass
	void PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext) override;
	void Render(Scene* pScene) override;
	void PostRender(ID3D11DeviceContext* pDeviceContext) override;
};

