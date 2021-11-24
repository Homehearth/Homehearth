#include "EnginePCH.h"
#include "ParticlePass.h"

void ParticlePass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
	//Binding genereal data
	DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	DC->IASetInputLayout(PM->m_ParticleInputLayout.Get());

	DC->PSSetSamplers(1, 1, PM->m_linearSamplerState.GetAddressOf());
	DC->VSSetShader(PM->m_ParticleVertexShader.Get(), nullptr, 0);
	DC->GSSetShader(PM->m_ParticleGeometryShader.Get(), nullptr, 0);
	DC->PSSetShader(PM->m_ParticlePixelShader.Get(), nullptr, 0);
	DC->CSSetShader(PM->m_ParticleComputeShader.Get(), nullptr, 0);

	DC->OMSetBlendState(PM->m_blendStateParticle.Get(), nullptr, 0xffffffff);

	DC->IASetVertexBuffers(0,1, &m_nullBuffer, &m_stride, &m_offset);
	DC->GSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());
	DC->CSSetShaderResources(18, 1, m_randomNumbersSRV.GetAddressOf());
}

void ParticlePass::CreateRandomNumbers()
{

	HRESULT hr;

	for (int i = 0; i < m_nrOfRandomNumbers; i++)
		m_randomNumbers.push_back((float)rand() / (RAND_MAX + 1) * (2.0 - (-2.0f)) + (-2.0f));

	D3D11_BUFFER_DESC bufferDescNR;
	ZeroMemory(&bufferDescNR, sizeof(D3D11_BUFFER_DESC));
	bufferDescNR.Usage = D3D11_USAGE_DEFAULT;
	bufferDescNR.ByteWidth = sizeof(UINT) * m_randomNumbers.size();
	bufferDescNR.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bufferDescNR.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDescNR.StructureByteStride = sizeof(float);

	D3D11_SUBRESOURCE_DATA dataEmitter;
	ZeroMemory(&dataEmitter, sizeof(D3D11_SUBRESOURCE_DATA));
	dataEmitter.pSysMem = m_randomNumbers.data();

	if (FAILED(hr = D3D11Core::Get().Device()->CreateBuffer(&bufferDescNR, &dataEmitter, m_randomNumbersBuffer.GetAddressOf())))
	{
		LOG_ERROR("Couldnt create m_randomNumbersBuffer");
	}


	D3D11_SHADER_RESOURCE_VIEW_DESC SRVRandomNunmbers;
	ZeroMemory(&SRVRandomNunmbers, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	SRVRandomNunmbers.Format = DXGI_FORMAT_UNKNOWN;
	SRVRandomNunmbers.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SRVRandomNunmbers.Buffer.NumElements = (UINT)m_randomNumbers.size();

	if (FAILED(hr = D3D11Core::Get().Device()->CreateShaderResourceView(m_randomNumbersBuffer.Get(), &SRVRandomNunmbers, m_randomNumbersSRV.GetAddressOf())))
	{
		LOG_ERROR("Couldnt create m_randomNumbersSRV");
	}
}

void ParticlePass::Initialize(ID3D11DeviceContext* pContextDevice, PipelineManager* pPipelineManager)
{
	m_contextDevice = pContextDevice;
	m_pipelineManager = pPipelineManager;

	m_constantBufferParticleUpdate.Create(D3D11Core::Get().Device());

	m_nrOfRandomNumbers = 100;
	CreateRandomNumbers();
}

void ParticlePass::Render(Scene* pScene)
{
	m_counter++;
	if (m_counter >= 10)
	{
		m_counter = 0;
	}

	std::vector<Entity> enities;
	pScene->ForEachComponent<comp::EmitterParticle>([&](Entity &entity, comp::EmitterParticle& particleEmitter) 
	{
			enities.push_back(entity);
	});
	
	for (auto entity : enities)
	{
		comp::EmitterParticle* emitter = entity.GetComponent<comp::EmitterParticle>();
		comp::Transform* transform = entity.GetComponent<comp::Transform>();

		//Constant buffer
		m_particleUpdate.emitterPosition = sm::Vector4(transform->position.x, transform->position.y, transform->position.z, 1.0f);
		m_particleUpdate.deltaTime = Stats::Get().GetFrameTime();
		m_particleUpdate.counter = m_counter;
		m_particleUpdate.lifeTime = (UINT)emitter->lifeTime;
		m_particleUpdate.particleSizeMulitplier = emitter->sizeMulitplier;

		m_constantBufferParticleUpdate.SetData(D3D11Core::Get().DeviceContext(), m_particleUpdate);
		ID3D11Buffer* cb = { m_constantBufferParticleUpdate.GetBuffer() };

		//Binding emitter speceific data
		D3D11Core::Get().DeviceContext()->CSSetConstantBuffers(8, 1 , &cb);
		D3D11Core::Get().DeviceContext()->CSSetUnorderedAccessViews(7, 1, emitter->particleUAV.GetAddressOf(), nullptr);

		D3D11Core::Get().DeviceContext()->Dispatch(emitter->nrOfParticles, 1, 1);
		D3D11Core::Get().DeviceContext()->CSSetUnorderedAccessViews(7, 1, &m_nullUAV, nullptr);

		D3D11Core::Get().DeviceContext()->PSSetShaderResources(1, 1, &emitter->texture->GetShaderView());
		D3D11Core::Get().DeviceContext()->PSSetShaderResources(7, 1, &emitter->opacityTexture->GetShaderView());
		D3D11Core::Get().DeviceContext()->VSSetShaderResources(17, 1, emitter->particleSRV.GetAddressOf());

		D3D11Core::Get().DeviceContext()->DrawInstanced(1, emitter->nrOfParticles, 0, 0);
		D3D11Core::Get().DeviceContext()->VSSetShaderResources(17, 1, &m_nullSRV);
	}
}

void ParticlePass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
	//Unbinding data
	DC->VSSetShaderResources(17, 1, &m_nullSRV);
	DC->GSSetConstantBuffers(1, 1, &m_nullBuffer);
	DC->CSSetUnorderedAccessViews(7, 1, &m_nullUAV, nullptr);
	DC->CSSetShaderResources(7, 1, &m_nullSRV);

	DC->GSSetShader(m_nullGS, nullptr, 0);
	DC->CSSetShader(m_nullCS, nullptr, 0);

	DC->OMSetBlendState(PM->m_blendStatepOpaque.Get(), 0, 0xffffffff);
}
