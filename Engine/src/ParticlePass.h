#pragma once
#include "IRenderPass.h"
#include "EnginePCH.h"

ALIGN16
struct ParticleUpdate 
{
	sm::Vector4 emitterPosition;
	float deltaTime;
};

class ParticlePass :public IRenderPass
{
private:

	UINT m_offset = sizeof(Particle_t);
	UINT m_stride = 0;

	dx::ConstantBuffer<ParticleUpdate> m_constantBuffer;
	ParticleUpdate m_particleUpdate;

	ID3D11Buffer* m_nullBuffer = nullptr;
	ID3D11ShaderResourceView* m_nullSRV = nullptr;
	ID3D11UnorderedAccessView* m_nullUAV = nullptr;
	ID3D11GeometryShader* m_nullGS = nullptr;
	ID3D11ComputeShader* m_nullCS = nullptr;

public:
	ParticlePass() = default;
	virtual ~ParticlePass() = default;

	void Initialize(ID3D11DeviceContext* pContextDevice, PipelineManager* pPipelineManager);

	void PreRender(Camera * pCam, ID3D11DeviceContext * pDeviceContext = D3D11Core::Get().DeviceContext()) override;

	void Render(Scene * pScene) override;

	void PostRender(ID3D11DeviceContext * pDeviceContext = D3D11Core::Get().DeviceContext()) override;
};

