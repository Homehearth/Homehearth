#include "ParticleSystem.h"

ParticleSystem::ParticleSystem()
{
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::Initialize(ID3D11Device* pDevice)
{
	m_pDevice = pDevice;
}

void ParticleSystem::InitializeParticles(Entity* emitterEntity)
{
	comp::EmitterParticle* emitter = emitterEntity->GetComponent<comp::EmitterParticle>();	
	std::vector<Particle_t> particles(emitter->nrOfParticles);

	for (int i = 0; i < emitter->nrOfParticles; i++)
	{
		Particle_t tempParticle;
		
		tempParticle.position = emitterEntity->GetComponent<comp::Transform>()->position;
		tempParticle.startPosition = tempParticle.position;
		tempParticle.type = emitter->type;
		tempParticle.size = { 1,1,1, };


		particles[i] =  tempParticle;
	}

	CreateBufferSRVUAV(particles, emitter);
}

bool ParticleSystem::CreateBufferSRVUAV(std::vector<Particle_t> particles, comp::EmitterParticle* emitter)
{
	D3D11_BUFFER_DESC descVert;
	ZeroMemory(&descVert, sizeof(descVert));
	descVert.Usage = D3D11_USAGE_DEFAULT;
	//assert(sizeof(Particle) % 16 == 0);				//Ser till att det �r 16 bitar
	descVert.ByteWidth = sizeof(Particle_t) * emitter->nrOfParticles;
	descVert.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	descVert.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	descVert.StructureByteStride = sizeof(Particle_t);

	D3D11_SUBRESOURCE_DATA dataVert;
	ZeroMemory(&dataVert, sizeof(dataVert));
	dataVert.pSysMem = particles.data();

	HRESULT hr = m_pDevice->CreateBuffer(&descVert, &dataVert, emitter->particleBuffer.GetAddressOf());

	if (FAILED(hr))
	{
		LOG_ERROR("Couldnt create particle buffer");
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
	ZeroMemory(&descSRV, sizeof(descSRV));
	descSRV.Format = DXGI_FORMAT_UNKNOWN;
	descSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	descSRV.BufferEx.NumElements = emitter->nrOfParticles;

	hr = m_pDevice->CreateShaderResourceView(emitter->particleBuffer.Get(), &descSRV, emitter->particleSRV.GetAddressOf());

	if (FAILED(hr))
	{
		LOG_ERROR("Couldnt create particle SRV");
		return false;
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV;
	descUAV.Format = DXGI_FORMAT_UNKNOWN;
	descUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	descUAV.Buffer.NumElements = emitter->nrOfParticles;
	descUAV.Buffer.FirstElement = 0;
	descUAV.Buffer.Flags = 0;

	hr = m_pDevice->CreateUnorderedAccessView(emitter->particleBuffer.Get(), &descUAV, emitter->particleUAV.GetAddressOf());

	if (FAILED(hr))
	{
		LOG_ERROR("Couldnt create particle UAV");
		return false;
	}

	return true;
}