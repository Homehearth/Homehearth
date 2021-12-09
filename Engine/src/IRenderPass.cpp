#include "EnginePCH.h"
#include "IRenderPass.h"

void IRenderPass::SetLights(Lights* light)
{
	m_lights = light;
	if (!m_lights->IsInitialize())
		m_lights->Initialize();
}

unsigned int IRenderPass::GetLightCount(TypeLight type) const
{
	unsigned int count = 0;
	for (auto& light : m_lights->GetLights())
	{
		if (light.type == type)
			count++;
	}
	return count;
}
