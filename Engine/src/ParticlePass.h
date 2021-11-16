#pragma once
#include "IRenderPass.h"
#include "EnginePCH.h"

class ParticlePass :public IRenderPass
{
private:
	ID3D11Buffer* m_nullVertexBuffer = nullptr;
	ID3D11ShaderResourceView* m_nullSRV = nullptr;
	ID3D11UnorderedAccessView* m_nullUAV = nullptr;

public:
	ParticlePass() = default;
	virtual ~ParticlePass() = default;

	void PreRender(Camera * pCam, ID3D11DeviceContext * pDeviceContext = D3D11Core::Get().DeviceContext()) override;

	void Render(Scene * pScene) override;

	void PostRender(ID3D11DeviceContext * pDeviceContext = D3D11Core::Get().DeviceContext()) override;
};

