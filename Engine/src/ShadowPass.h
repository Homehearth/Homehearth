#pragma once
#include "IRenderPass.h"


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
	
	uint32_t m_shadowSize;

	ComPtr<ID3D11DepthStencilView> CreateDepthView(uint32_t index);
	ComPtr<ID3D11Buffer> CreateLightBuffer(light_t light);

	void RenderWithImmidiateContext(Scene* pScene, const ShadowSection& shadow);

	void CreateMap(uint32_t arraySize);
public:
	static camera_Matrix_t GetLightMatrix(light_t light, sm::Vector3 direction);

	ShadowPass();
	~ShadowPass();

	// Does this instead of default constructor.
	void StartUp();

	// Finalize and setup the shadow map.
	void SetupMap();

	// Inherited via IRenderPass
	virtual void PreRender(Camera* pCam = nullptr, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
	virtual void Render(Scene* pScene) override;
	virtual void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
	
	void UpdateLightBuffer(ID3D11DeviceContext* context, ID3D11Buffer* buffer, light_t light, sm::Vector3 direction);

	void SetShadowMapSize(uint32_t size);
	uint32_t GetShadowMapSize() const;

	void ImGuiShowTextures() override;
};