#pragma once
#pragma once
#include "IRenderPass.h"

//--------------------------------------
// Forward Rendering, or GeometryPass.
//--------------------------------------
class TextureEffectPass : public IRenderPass
{
private:

	const int MAX_PIXELS = 256 * 256;
	float clearColor[4] = { 0.f,0.f,0.f,0.f };
	texture_effect_t m_CBuffer;
	ID3D11ShaderResourceView* resources[5] =
	{
		PM->m_SRV_TextureEffectBlendMap.Get(),
		PM->m_SRV_TextureEffectWaterEdgeMap.Get(),
		PM->m_SRV_TextureEffectWaterFloorMap.Get(),
		PM->m_SRV_TextureEffectWaterMap.Get(),
		PM->m_SRV_TextureEffectWaterNormalMap.Get()
	};

	ID3D11RenderTargetView* targets[5] =
	{
		PM->m_RTV_TextureEffectBlendMap.Get(),
		PM->m_RTV_TextureEffectWaterEdgeMap.Get(),
		PM->m_RTV_TextureEffectWaterFloorMap.Get(),
		PM->m_RTV_TextureEffectWaterMap.Get(),
		PM->m_RTV_TextureEffectWaterNormalMap.Get()
	};

public:
	TextureEffectPass() = default;
	virtual ~TextureEffectPass() = default;

	void PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

	void Render(Scene* pScene) override;

	void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
};

