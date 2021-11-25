#pragma once
#pragma once
#pragma once
#include "IRenderPass.h"

//--------------------------------------
// Water effect on the water mesh.
//--------------------------------------
class WaterEffectPass : public IRenderPass
{
private:

	const int m_MAX_PIXELS = 256 * 256;
	float m_clearColor[4] = { 0.f,0.f,0.f,0.f };
	texture_effect_t m_CBuffer;

public:
	WaterEffectPass() = default;
	virtual ~WaterEffectPass() = default;

	void PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

	void Render(Scene* pScene) override;

	void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

};

