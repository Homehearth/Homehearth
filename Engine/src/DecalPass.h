#pragma once
#include "IRenderPass.h"

struct DecalInfoBuffer
{
	dx::XMFLOAT4 info;
	dx::XMMATRIX projection;
};

class DecalPass : public IRenderPass
{
private:

	ID3D11Buffer* m_buffer;
	dx::ConstantBuffer<DecalInfoBuffer> m_infoBuffer;
	ID3D11ShaderResourceView* m_shaderView;
	std::vector<sm::Matrix> m_matrices;
	DecalInfoBuffer m_decalInfoBuffer;

	std::shared_ptr<RTexture> tempTexture = nullptr;

	void CreateBuffer();

public:

	void Create();
	DecalPass();
	~DecalPass();

	// Inherited via IRenderPass
	virtual void PreRender(Camera* pCam = nullptr, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
	virtual void Render(Scene* pScene) override;
	virtual void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
};