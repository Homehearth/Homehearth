#pragma once
#include "EnginePCH.h"

enum class PARTICLEMODE : UINT
{
	BLOOD,
	LEAF,
	WATERSPLASH,
	SMOKE,
	SPARKLES, 
	RAIN
};

class ParticleSystem
{
private:

	RTexture* texture = nullptr;
	UINT type = 0U;
	UINT nrOfParticles = 0;
	std::vector<Particle_t> particles;

	ID3D11Device* m_pDevice;

public:
	ParticleSystem();
	~ParticleSystem();

	void Initialize(ID3D11Device* pDevice);
	void InitializeParticles(Entity* emitterEntity);

	bool CreateBufferSRVUAV(std::vector<Particle_t> particles, comp::EmitterParticle* emitter);

};