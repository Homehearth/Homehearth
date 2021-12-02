#pragma once
#include "IRenderPass.h"
#include "EnginePCH.h"

ALIGN16
struct ParticleUpdate 
{
	sm::Vector4 emitterPosition;;
	float deltaTime;
	UINT counter;
	float lifeTime;
	float particleSizeMulitplier;
	float speed;
};
ALIGN16
struct ParticleModePUpdate 
{
	PARTICLEMODE type;
};

class ParticlePass :public IRenderPass
{
private:

	UINT m_offset = sizeof(Particle_t);
	UINT m_stride = 0;

	UINT								m_counter = 0;
	UINT								m_nrOfRandomNumbers = 0;
	std::vector<float>					m_randomNumbers;

	dx::ConstantBuffer<ParticleUpdate>	m_constantBufferParticleUpdate;
	dx::ConstantBuffer<ParticleModePUpdate>	m_constantBufferParticleMode;
	ParticleUpdate						m_particleUpdate;
	ParticleModePUpdate					m_particleModeUpdate;

	ComPtr<ID3D11ShaderResourceView>	m_randomNumbersSRV;
	ComPtr<ID3D11Buffer>				m_randomNumbersBuffer;

	ID3D11Buffer*						m_nullBuffer = nullptr;
	ID3D11ShaderResourceView*			m_nullSRV = nullptr;
	ID3D11UnorderedAccessView*			m_nullUAV = nullptr;
	ID3D11GeometryShader*				m_nullGS = nullptr;
	ID3D11ComputeShader*				m_nullCS = nullptr;

	void CreateRandomNumbers();

public:
	Skybox* m_skyboxRef = nullptr;

	ParticlePass() = default;
	virtual ~ParticlePass() = default;

	void Initialize(ID3D11DeviceContext* pContextDevice, PipelineManager* pPipelineManager);

	void PreRender(Camera * pCam, ID3D11DeviceContext * pDeviceContext = D3D11Core::Get().DeviceContext()) override;

	void Render(Scene * pScene) override;

	void PostRender(ID3D11DeviceContext * pDeviceContext = D3D11Core::Get().DeviceContext()) override;
};

