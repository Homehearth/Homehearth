#pragma once
#include "EnginePCH.h"

class ParticleSystem
{
private:
	ID3D11Device* m_pDevice;
	Particle_t m_tempParticle;

	/*	m_tempParticle.position += (float)rand() / (RAND_MAX + 1.f) * (max - (min)) + (min);*/
	void RandomAddPosition(float min, float max);

	/*	m_tempParticle.velocity = (float)rand() / (RAND_MAX + 1.f) * (max - (min)) + (min);*/
	void RandomSetVelocity(float min, float max);

public:
	ParticleSystem();
	~ParticleSystem();

	void Initialize(ID3D11Device* pDevice);
	void InitializeParticles(entt::registry& reg, entt::entity ent);

	bool CreateBufferSRVUAV(std::vector<Particle_t> particles, comp::EmitterParticle* emitter);

};