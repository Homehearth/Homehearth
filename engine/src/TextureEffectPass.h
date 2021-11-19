#pragma once
#pragma once
#include "IRenderPass.h"

//--------------------------------------
// Forward Rendering, or GeometryPass.
//--------------------------------------
class TextureEffectPass : public IRenderPass
{
private:

	const int m_MAX_PIXELS = 256 * 256;
	float m_clearColor[4] = { 0.f,0.f,0.f,0.f };
	texture_effect_t m_CBuffer;

public:
	TextureEffectPass() = default;
	virtual ~TextureEffectPass() = default;

	void PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

	void Render(Scene* pScene) override;

	void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

};

