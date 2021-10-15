#include "EnginePCH.h"
#include "IRenderPass.h"

void IRenderPass::SetLights(Lights* light)
{
	m_lights = light;
	if (!m_lights->IsInitialize())
		m_lights->Initialize();
}
