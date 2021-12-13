#pragma once
#include "Shader.h"

/*
 * This class holds the D3D11-specific resources used in different passes.
 */
class PipelineManager
{
	struct ResourceAccessView
	{
		ComPtr<ID3D11Buffer> buffer;
		ComPtr<ID3D11ShaderResourceView> srv;
		ComPtr<ID3D11UnorderedAccessView> uav;
	};

	struct TextureStencilView
	{
		ComPtr<ID3D11Texture2D>				texture;
		ComPtr<ID3D11DepthStencilView>		dsv;
		ComPtr<ID3D11ShaderResourceView>	srv;
	};

private:
	Window*								m_window;
	D3D11Core*							m_d3d11;
	ID3D11DeviceContext*				m_context;
	
public:
	PipelineManager();
	virtual ~PipelineManager() = default;

	// Initialize PipelineManager.
	void Initialize(Window* pWindow, ID3D11DeviceContext* context = D3D11Core::Get().DeviceContext());

	// PUBLIC AVAILABLE DATA.
	ComPtr<ID3D11RenderTargetView>		m_backBuffer;
	ComPtr<ID3D11RenderTargetView>		m_renderTargetView;

	ComPtr<ID3D11UnorderedAccessView> 	m_backBufferAccessView;
	ComPtr<ID3D11RenderTargetView>	  	m_bloomTargetView;
	ComPtr<ID3D11Texture2D>			  	m_bloomTexture;
	ComPtr<ID3D11UnorderedAccessView> 	m_bloomAccessView;

	ComPtr<ID3D11Texture2D>				m_depthStencilTexture;
	ComPtr<ID3D11DepthStencilView>		m_depthStencilView;
	ComPtr<ID3D11ShaderResourceView>	m_depthBufferSRV;

	ComPtr<ID3D11Texture2D>				m_debugDepthStencilTexture;
	ComPtr<ID3D11DepthStencilView>		m_debugDepthStencilView;
	ComPtr<ID3D11ShaderResourceView>	m_debugDepthBufferSRV;

	TextureStencilView					m_depth; // used for depth pass.
	
	ComPtr<ID3D11DepthStencilState>		m_depthStencilStateLess;
	ComPtr<ID3D11DepthStencilState>		m_depthStencilStateLessOrEqual;
	ComPtr<ID3D11DepthStencilState>		m_depthStencilStateGreater;
	ComPtr<ID3D11DepthStencilState>		m_depthStencilStateLessOrEqualEnableDepthWrite;
	
	ComPtr<ID3D11RasterizerState>		m_rasterStateBackCulling;
	ComPtr<ID3D11RasterizerState>		m_rasterStateNoCulling;
	ComPtr<ID3D11RasterizerState>		m_rasterStateFrontCulling;
	ComPtr<ID3D11RasterizerState>		m_rasterStateWireframe;

	ComPtr<ID3D11BlendState>			m_blendStatepOpaque;
	ComPtr<ID3D11BlendState>			m_blendStatepDepthOnlyAlphaTest;
	ComPtr<ID3D11BlendState>			m_blendStateDepthOnlyAlphaToCoverage;
	ComPtr<ID3D11BlendState>			m_blendStateAlphaBlending;
	ComPtr<ID3D11BlendState>			m_blendStateParticle;
	ComPtr<ID3D11BlendState>			m_blendOn;
	ComPtr<ID3D11BlendState>			m_blendOff;
	ComPtr<ID3D11BlendState>			m_alphaBlending;

	ComPtr<ID3D11SamplerState>			m_linearSamplerState;		//Low settings
	ComPtr<ID3D11SamplerState>			m_linearClampSamplerState;
	ComPtr<ID3D11SamplerState>			m_anisotropicSamplerState;	//High settings
	ComPtr<ID3D11SamplerState>			m_pointSamplerState;
	ComPtr<ID3D11SamplerState>			m_cubemapSamplerState;

	ComPtr<ID3D11InputLayout>			m_defaultInputLayout;
	ComPtr<ID3D11InputLayout>			m_animationInputLayout;
	ComPtr<ID3D11InputLayout>			m_skyboxInputLayout;
	ComPtr<ID3D11InputLayout>			m_ParticleInputLayout;

	Shaders::VertexShader				m_defaultVertexShader;
	Shaders::VertexShader				m_depthPassVertexShader;
	Shaders::VertexShader				m_animationVertexShader;
	Shaders::VertexShader				m_skyboxVertexShader;
	Shaders::VertexShader				m_ParticleVertexShader;
	Shaders::VertexShader				m_bloomVertexShader;

	Shaders::PixelShader				m_defaultPixelShader;
	Shaders::PixelShader				m_debugPixelShader;
	Shaders::PixelShader				m_depthPassPixelShader;
	Shaders::PixelShader				m_skyboxPixelShader;
	Shaders::PixelShader				m_ParticlePixelShader;
	Shaders::PixelShader				m_bloomPixelShader;

	//Shadows
	Shaders::VertexShader				m_paraboloidVertexShader;
	Shaders::VertexShader				m_paraboloidAnimationVertexShader;
	Shaders::PixelShader				m_shadowPixelShader;

	Shaders::ComputeShader				m_ParticleComputeShader;
	Shaders::GeometryShader				m_ParticleGeometryShader;

	Shaders::ComputeShader				m_lightCullingShader;
	Shaders::ComputeShader				m_computeFrustumsShader;
	Shaders::ComputeShader				m_blurComputeShader;
	Shaders::ComputeShader				m_dofComputeShader;
	Shaders::ComputeShader				m_bloomComputeShader;

	D3D11_VIEWPORT						m_viewport;

	bool CreateStructuredBuffer(ComPtr<ID3D11Buffer>& buffer, void* data, unsigned int byteStride,
		unsigned int arraySize, ComPtr<ID3D11UnorderedAccessView>& uav);
	bool CreateCopyBuffer(ID3D11Buffer** buffer, unsigned int byteStride, unsigned int arraySize);
	bool CreateStructuredBuffer(void* data, unsigned int byteStride, unsigned int arraySize, ResourceAccessView& rav);
	void BindStructuredBuffer(size_t startSlot, size_t numUAV, ID3D11Buffer* buffer, void* data, ID3D11UnorderedAccessView** uav);
	void SetCullBack(bool cullNone, ID3D11DeviceContext* pDeviceContext);

	//
	// Forward+ Resources.
	//
	const uint32_t AVERAGE_OVERLAPPING_LIGHTS_PER_TILE = 20u;
	uint32_t m_numFrustums;

	dispatch_params_t m_dispatchParams;
	screen_view_params_t m_screenToViewParams;
	DirectX::ConstantBuffer<dispatch_params_t> m_dispatchParamsCB;
	DirectX::ConstantBuffer<screen_view_params_t> m_screenToViewParamsCB;

	std::vector<frustum_t> m_frustums_data;

	std::vector<UINT> opaq_LightIndexCounter_data;
	std::vector<UINT> trans_LightIndexCounter_data;
	std::vector<UINT> opaq_LightIndexList_data;
	std::vector<UINT> trans_LightIndexList_data;

	// SRV: Precomputed frustums used in LightCulling_cs.
	// UAV: Used in ComputerFrustums_cs to store computed frustums.
	ResourceAccessView m_frustums;

	ResourceAccessView opaq_LightIndexCounter;
	ResourceAccessView trans_LightIndexCounter;

	// Count of values stored in a light index list.
	// Size is based the expected average number of overlapping lights per tile.
	ResourceAccessView opaq_LightIndexList;
	ResourceAccessView trans_LightIndexList;

	// Stores an offset.
	ResourceAccessView opaq_LightGrid;
	ResourceAccessView trans_LightGrid;

	ResourceAccessView m_heatMap;
	std::shared_ptr<RTexture> m_lightCountHeatMap;

	UINT								m_windowWidth;
	UINT								m_windowHeight;

	// View space frustums for the grid cells used in ForwardPlus rendering.
	ComPtr<ID3D11Buffer>				m_gridFrustum;
	ComPtr<ID3D11ShaderResourceView>	m_gridFrustumSRV;
	
	// Nikkis stuff
	Shaders::VertexShader				m_textureEffectVertexShader;  // Dummy.
	Shaders::ComputeShader				m_textureEffectComputeShader; // Water refraction effect compute shader.
	Shaders::PixelShader				m_textureEffectPixelShader;   // Dummy.

	Shaders::VertexShader				m_WaterEffectVertexShader;  // Dummy.
	Shaders::ComputeShader				m_WaterEffectComputeShader; // Water effect compute shader.
	Shaders::PixelShader				m_WaterEffectPixelShader;   // Dummy.

	ComPtr<ID3D11Buffer>				m_textureEffectConstantBuffer;
	//ComPtr<ID3D11Buffer>              m_deltaTimeBuffer;

	ComPtr<ID3D11ShaderResourceView>	m_SRV_TextureEffectBlendMap;
	ComPtr<ID3D11ShaderResourceView>	m_SRV_TextureEffectWaterMap;
	ComPtr<ID3D11ShaderResourceView>	m_SRV_TextureEffectWaterFloorMap;
	//ComPtr<ID3D11ShaderResourceView>  m_SRV_TextureEffectWaterEdgeMap;
	ComPtr<ID3D11ShaderResourceView>	m_SRV_TextureEffectWaterNormalMap;

	ComPtr<ID3D11UnorderedAccessView>	m_UAV_TextureEffectBlendMap;
	ComPtr<ID3D11UnorderedAccessView>	m_UAV_TextureEffectWaterMap;
	ComPtr<ID3D11UnorderedAccessView>	m_UAV_TextureEffectWaterFloorMap;
	//ComPtr<ID3D11UnorderedAccessView> m_UAV_TextureEffectWaterEdgeMap;
	ComPtr<ID3D11UnorderedAccessView>	m_UAV_TextureEffectWaterNormalMap;

	std::shared_ptr<RModel>				m_WaterModel;
	std::shared_ptr<RModel>				m_WaterEdgeModel;
	std::shared_ptr<RModel>				m_WaterFloorModel;

	ComPtr<ID3D11Texture2D>				m_WaterAlbedoMap;
	ComPtr<ID3D11Texture2D>				m_WaterNormalMap;
	ComPtr<ID3D11Texture2D>				m_WaterEdgeAlbedoMap;
	ComPtr<ID3D11Texture2D>				m_WaterFloorAlbedoMap;
	ComPtr<ID3D11Texture2D>				m_WaterBlendAlbedoMap; //This one is a stand alone texture and is not found on a model.

	std::shared_ptr<RTexture>			m_ModdedWaterAlbedoMap;
	std::shared_ptr<RTexture>			m_ModdedWaterNormalMap;
	std::shared_ptr<RTexture>			m_ModdedWaterEdgeAlbedoMap;
	std::shared_ptr<RTexture>			m_ModdedWaterFloorAlbedoMap;
	std::shared_ptr<RTexture>			m_ModdedWaterBlendAlbedoMap;

private:
	// INITIALIZE METHODS.
	
	bool CreateRenderTargetView();
	bool CreateDepthStencilStates();
	bool CreateDepthBuffer();
	bool CreateRasterizerStates();
	bool CreateSamplerStates();
	bool CreateBlendStates();
	bool CreateShaders();
	bool CreateInputLayouts();
	void SetViewport();

	bool CreateTextureEffectConstantBuffer();
	bool CreateTextureEffectResources();
	bool CreateDepth();
	bool CreateForwardBlendStates();
	bool CreateHeatMapTexture();
};


