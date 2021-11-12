#include "ParticleSystem.h"

ParticleSystem::ParticleSystem()
{
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::Initialize(Entity* emitterEntity)
{
	comp::EmitterParticle* emitter = emitterEntity->GetComponent<comp::EmitterParticle>();	
	std::vector<Particle_t> particles;



	for (int i = 0; i < emitter->nrOfParticles; i++)
	{
		Particle_t tempParticle;
		
		tempParticle.position = emitterEntity->GetComponent<comp::Transform>()->position;
		tempParticle.startPosition = tempParticle.position;
		tempParticle.type = emitter->type;
		tempParticle.size = { 1,1,1, };


		particles.push_back(tempParticle);
	}

	emitter->particles = particles;

}
