#include "EnginePCH.h"
#include "ParticleSystem.h"

ParticleSystem::ParticleSystem()
:m_pDevice(nullptr)
{
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::Initialize(ID3D11Device* pDevice)
{
	m_pDevice = pDevice;
}

void ParticleSystem::InitializeParticles(entt::registry& reg, entt::entity ent)
{
	comp::EmitterParticle* emitter = reg.try_get<comp::EmitterParticle>(ent);
	comp::Transform* t = reg.try_get<comp::Transform>(ent);

	if (!t || !emitter)
		return;

	sm::Vector3 entityPosition = t->position;
	entityPosition = sm::Vector3{ entityPosition.x + emitter->positionOffset.x, entityPosition.y + emitter->positionOffset.y, entityPosition.z + emitter->positionOffset.z };

	std::vector<Particle_t> particles(emitter->nrOfParticles);
	for (UINT i = 0; i < emitter->nrOfParticles; i++)
	{
		m_tempParticle.position = sm::Vector4(entityPosition.x , entityPosition.y, entityPosition.z, 1.f);
		m_tempParticle.type = emitter->type;
		m_tempParticle.size = { 1 , 1 };
		m_tempParticle.color = { 1,1,1,1 };
		m_tempParticle.life = 0;
		m_tempParticle.velocity = {0,0,0,0};

		switch (m_tempParticle.type)
		{
		case ParticleMode::BLOOD:
		{
			RandomSetVelocity(-2.0f, 2.0f);
			m_tempParticle.size = { emitter->sizeMulitplier , emitter->sizeMulitplier };
			break;
		}

		case ParticleMode::LEAF:
		{
			break;
		}
		case ParticleMode::WATERSPLASH:
		{
			RandomAddPosition(-1.0f, 1.0f);
			RandomSetVelocity(-2.0f, 2.0f);
			m_tempParticle.size = { emitter->sizeMulitplier , emitter->sizeMulitplier };
			m_tempParticle.color = { 0.f, 0.f, 0.5f, 0.5f };
			break;
		}
		case ParticleMode::SMOKEPOINT:
		{
			break;
		}
		case ParticleMode::SMOKEAREA:
		{
			RandomAddPosition(-1.0f, 1.0f);
			break;
		}
		case ParticleMode::SPARKLES:
		{
			break;
		}
		case ParticleMode::RAIN:
		{
			RandomAddPositionXYZ(sm::Vector2(-200.f, 250.f), sm::Vector2(0 , 0), sm::Vector2(-200.f, 250.f ));
			m_tempParticle.size = { emitter->sizeMulitplier , emitter->sizeMulitplier };
			break;
		}
		case ParticleMode::DUST:
		{
			break;
		}
		case ParticleMode::MAGEHEAL:
		{
			RandomSetVelocity(-1.0f, 1.0f);

			m_tempParticle.velocity.y = 0.0f;
			m_tempParticle.velocity.Normalize();
			m_tempParticle.size = sm::Vector2(emitter->sizeMulitplier, emitter->sizeMulitplier);
			m_tempParticle.position.y = (float)rand() / (RAND_MAX + 1.f) * (10.0f - (1.0f)) + (1.0f);
			break;
		}
		case ParticleMode::MAGERANGE:
		{
			RandomAddPosition(-1.0f, 1.0f);
			RandomSetVelocity(-2.0f, 2.0f);
			m_tempParticle.size = { emitter->sizeMulitplier , emitter->sizeMulitplier };
			break;
		}
		}

		particles[i] =  m_tempParticle;
	}

	CreateBufferSRVUAV(particles, emitter);
}

bool ParticleSystem::CreateBufferSRVUAV(std::vector<Particle_t> particles, comp::EmitterParticle* emitter)
{
	D3D11_BUFFER_DESC descVert;
	ZeroMemory(&descVert, sizeof(descVert));
	descVert.Usage = D3D11_USAGE_DEFAULT;
	assert(sizeof(Particle_t) % 16 == 0);
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

void ParticleSystem::RandomAddPosition(float min, float max)
{
	m_tempParticle.position.x += (float)rand() / (RAND_MAX + 1.f) * (max - (min)) + (min);
	m_tempParticle.position.y += (float)rand() / (RAND_MAX + 1.f) * (max - (min)) + (min);
	m_tempParticle.position.z += (float)rand() / (RAND_MAX + 1.f) * (max - (min)) + (min);
}

void ParticleSystem::RandomAddPositionXYZ(sm::Vector2 minMaxX, sm::Vector2 minMaxY, sm::Vector2 minMaxZ)
{
	m_tempParticle.position.x += (float)rand() / (RAND_MAX + 1.f) * (minMaxX.y - (minMaxX.x)) + (minMaxX.x);
	m_tempParticle.position.y += (float)rand() / (RAND_MAX + 1.f) * (minMaxY.y - (minMaxY.x)) + (minMaxY.x);
	m_tempParticle.position.z += (float)rand() / (RAND_MAX + 1.f) * (minMaxZ.y - (minMaxZ.x)) + (minMaxZ.x);
}

void ParticleSystem::RandomSetVelocity(float min, float max)
{
	m_tempParticle.velocity.x = (float)rand() / (RAND_MAX + 1.f) * (max - (min)) + (min);
	m_tempParticle.velocity.y = (float)rand() / (RAND_MAX + 1.f) * (max - (min)) + (min);
	m_tempParticle.velocity.z = (float)rand() / (RAND_MAX + 1.f) * (max - (min)) + (min);
}