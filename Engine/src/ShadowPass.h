#pragma once
#include "IRenderPass.h"

constexpr uint64_t SHADOW_SIZE = 2048;


// Single shadow section of shadow map.
struct ShadowSection
{
	ComPtr<ID3D11DepthStencilView> shadowDepth[2];
	ComPtr<ID3D11Buffer> lightBuffer;
	light_t* pLight;
};

class ShadowPass : public IRenderPass
{
private:
	D3D11_VIEWPORT m_viewport;
	// The complete map of all shadows.
	struct ShadowMap
	{
		ComPtr<ID3D11Texture2D> shadowTexture;
		ComPtr<ID3D11ShaderResourceView> shadowView;
		unsigned int amount = 0;
	}m_shadowMap;

	// Single shadow section of shadow map.
	

	std::vector<ShadowSection> m_shadows;

	//ID3D11DepthStencilView*& EmplaceInMap(const unsigned int& index);


	ComPtr<ID3D11DepthStencilView> CreateDepthView(uint32_t index);
	ComPtr<ID3D11Buffer> CreateLightBuffer(light_t light);

	void RenderWithImmidiateContext(Scene* pScene, const ShadowSection& shadow);

public:
	static camera_Matrix_t GetLightMatrix(light_t light, sm::Vector3 direction);

	ShadowPass();
	~ShadowPass();

	// Finalize and setup the shadow map.
	void SetupMap(uint32_t arraySize);

	// Inherited via IRenderPass
	virtual void PreRender(Camera* pCam = nullptr, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
	virtual void Render(Scene* pScene) override;
	virtual void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
	
	void UpdateLightBuffer(ID3D11DeviceContext* context, ID3D11Buffer* buffer, light_t light, sm::Vector3 direction);

	void ImGuiShowTextures() override;
};