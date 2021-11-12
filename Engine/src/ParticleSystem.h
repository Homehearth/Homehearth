#pragma once
#include "EnginePCH.h"

enum class PARTICLEMODE : UINT
{
	BLOOD,
	LEAF,
	WASTER,
	SMOKE,
	SPARKELS, 
	RAIN
};

class ParticleSystem
{
private:

public:
	ParticleSystem();
	~ParticleSystem();

	void Initialize(Entity* emitterEntity);

};

