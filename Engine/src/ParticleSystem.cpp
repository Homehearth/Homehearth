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
	{
		return;
	}

	sm::Vector3 entityPosition = t->position;
	entityPosition = sm::Vector3{ entityPosition.x + emitter->positionOffset.x, entityPosition.y + emitter->positionOffset.y, entityPosition.z + emitter->positionOffset.z };



	std::vector<Particle_t> particles(emitter->nrOfParticles);
	for (UINT i = 0; i < emitter->nrOfParticles; i++)
	{
		Particle_t tempParticle;
		tempParticle.position = sm::Vector4(entityPosition.x , entityPosition.y, entityPosition.z, 1.f);
		tempParticle.type = emitter->type;
		tempParticle.size = { 1 , 1 };
		tempParticle.color = { 1,1,1,1 };
		tempParticle.life = 0;
		tempParticle.velocity = {0,0,0,0};

		if (tempParticle.type == PARTICLEMODE::BLOOD || tempParticle.type == PARTICLEMODE::MAGERANGE || tempParticle.type == PARTICLEMODE::WATERSPLASH)
		{
			tempParticle.velocity.x = (float)rand() / (RAND_MAX + 1.f) * (2.0f - (-2.0f)) + (-2.0f);
			tempParticle.velocity.y = (float)rand() / (RAND_MAX + 1.f) * (2.0f - (-2.0f)) + (-2.0f);
			tempParticle.velocity.z = (float)rand() / (RAND_MAX + 1.f) * (2.0f - (-2.0f)) + (-2.0f);
		}
		else if (tempParticle.type == PARTICLEMODE::WATERSPLASH)
		{
			tempParticle.color = { 0.f, 0.f, 0.5f, 0.5f };
		}
		else if (tempParticle.type == PARTICLEMODE::SMOKEAREA || tempParticle.type == PARTICLEMODE::MAGERANGE || tempParticle.type == PARTICLEMODE::WATERSPLASH)
		{
			tempParticle.position.x += (float)rand() / (RAND_MAX + 1.f) * (1.0f - (-1.0f)) + (-1.0f);
			tempParticle.position.y += (float)rand() / (RAND_MAX + 1.f) * (1.0f - (-1.0f)) + (-1.0f);
			tempParticle.position.z += (float)rand() / (RAND_MAX + 1.f) * (1.0f - (-1.0f)) + (-1.0f);
		}
		else if (tempParticle.type == PARTICLEMODE::MAGEHEAL)
		{
			float xRandSame = (float)rand() / (RAND_MAX + 1.f) * (1.0f - (-1.0f)) + (-1.0f);
			float yRandSame = (float)rand() / (RAND_MAX + 1.f) * (1.0f - (-1.0f)) + (-1.0f);
			float zRandSame = (float)rand() / (RAND_MAX + 1.f) * (1.0f - (-1.0f)) + (-1.0f);
			sm::Vector3 vel = sm::Vector3(xRandSame, 0.0f, yRandSame);
			vel.Normalize();
			tempParticle.velocity = sm::Vector4( vel.x, vel.y, vel.z, 0.0f );

			tempParticle.size = sm::Vector2(emitter->sizeMulitplier, emitter->sizeMulitplier);
			tempParticle.position.y = (float)rand() / (RAND_MAX + 1.f) * (10.0f - (1.0f)) + (1.0f);
		}
		

		particles[i] =  tempParticle;
	}

	CreateBufferSRVUAV(particles, emitter);
}

bool ParticleSystem::CreateBufferSRVUAV(std::vector<Particle_t> particles, comp::EmitterParticle* emitter)
{
	D3D11_BUFFER_DESC descVert;
	ZeroMemory(&descVert, sizeof(descVert));
	descVert.Usage = D3D11_USAGE_DEFAULT;
	//assert(sizeof(Particle) % 16 == 0);				//Ser till att det är 16 bitar
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
