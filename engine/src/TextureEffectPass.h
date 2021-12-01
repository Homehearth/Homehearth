#pragma once
#pragma once
#include "IRenderPass.h"

//--------------------------------------
// Water refraction effect on the water floor and water edge meshes.
//--------------------------------------
class TextureEffectPass : public IRenderPass
{
private:

	texture_effect_t m_CBuffer;

public:
	TextureEffectPass() = default;
	virtual ~TextureEffectPass() = default;

	void PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

	void Render(Scene* pScene) override;

	void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

};

