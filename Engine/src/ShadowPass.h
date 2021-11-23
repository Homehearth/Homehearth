#pragma once
#include "IRenderPass.h"

constexpr uint64_t SHADOW_SIZE = 4096;

class ShadowPass : public IRenderPass
{
private:

	// The complete map of all shadows.
	struct ShadowMap
	{
		ID3D11Texture2D* shadowTexture = nullptr;
		ID3D11ShaderResourceView* shadowView = nullptr;
		unsigned int amount = 0;
	}m_shadowMap;

	// Single shadow section of shadow map.
	struct ShadowSection
	{
		ID3D11DepthStencilView* shadowDepth = nullptr;
		comp::Light light;
	};

	std::vector<ShadowSection> m_shadows;

	//ID3D11DepthStencilView*& EmplaceInMap(const unsigned int& index);

public:

	ShadowPass();
	~ShadowPass();

	// Register a shadow in the shadow map.
	void CreateShadow(const comp::Light& light);

	// Finalize and setup the shadow map.
	void SetupMap();

	// Inherited via IRenderPass
	virtual void PreRender(Camera* pCam = nullptr, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
	virtual void Render(Scene* pScene) override;
	virtual void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
};