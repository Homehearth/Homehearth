#pragma once
#include "IRenderPass.h"

class DecalPass : public IRenderPass
{
private:

	ID3D11Buffer* m_buffer;
	std::vector<sm::Matrix> m_matrices;

	void CreateBuffer();

public:

	DecalPass();
	~DecalPass();

	// Inherited via IRenderPass
	virtual void PreRender(Camera* pCam = nullptr, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
	virtual void Render(Scene* pScene) override;
	virtual void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
};