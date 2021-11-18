#pragma once
#pragma once
#include "IRenderPass.h"

//--------------------------------------
// Forward Rendering, or GeometryPass.
//--------------------------------------
class TextureEffectPass : public IRenderPass
{
private:

	std::shared_ptr<RModel> m_WaterModel;
	std::shared_ptr<RModel> m_WaterEdgeModel;
	std::shared_ptr<RModel> m_WaterFloorModel;

	std::vector<sm::Vector2> m_WaterUV;
	std::vector<sm::Vector2> m_WaterEdgeUV;
	std::vector<sm::Vector2> m_WaterFloorUV;

	std::shared_ptr<RTexture> m_WaterAlbedoMap;
	std::shared_ptr<RTexture> m_WaterNormalMap;
	std::shared_ptr<RTexture> m_WaterEdgeAlbedoMap;
	std::shared_ptr<RTexture> m_WaterFloorAlbedoMap;

	const int m_MAX_PIXELS = 256 * 256;
	float m_clearColor[4] = { 0.f,0.f,0.f,0.f };
	texture_effect_t m_CBuffer;

public:
	TextureEffectPass() = default;
	virtual ~TextureEffectPass() = default;

	void PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

	void Render(Scene* pScene) override;

	void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

	void SetResources();
};

