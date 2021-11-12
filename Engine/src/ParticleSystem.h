#pragma once
#include "EnginePCH.h"

struct Particle_t
{
	sm::Vector3 position;
	sm::Vector2 size;
	sm::Vector3 color;
	INT EmitterID;
};

class ParticleSystem
{
private:
public:
	ParticleSystem();
	~ParticleSystem();

	void Initialize(UINT nrOfParticles, comp::EmitterParticle);

};

