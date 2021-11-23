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

private:
	Window*							m_window;
	D3D11Core*						m_d3d11;
	ID3D11DeviceContext*			m_context;

public:
	PipelineManager();
	virtual ~PipelineManager() = default;

	// Initialize PipelineManager.
	void Initialize(Window* pWindow, ID3D11DeviceContext* context = D3D11Core::Get().DeviceContext());

	// PUBLIC AVAILABLE DATA.
	ComPtr<ID3D11RenderTargetView>	m_backBuffer;

	ComPtr<ID3D11Texture2D>			m_depthStencilTexture;
	ComPtr<ID3D11DepthStencilView>	m_depthStencilView;
	ComPtr<ID3D11ShaderResourceView>m_depthBufferSRV;

	ComPtr<ID3D11Texture2D>			m_debugDepthStencilTexture;
	ComPtr<ID3D11DepthStencilView>	m_debugDepthStencilView;
	ComPtr<ID3D11ShaderResourceView>m_debugDepthBufferSRV;

	ComPtr<ID3D11DepthStencilState>	m_depthStencilStateLess;
	ComPtr<ID3D11DepthStencilState>	m_depthStencilStateLessEqual;
	ComPtr<ID3D11DepthStencilState> m_depthStencilStateGreater;
	ComPtr<ID3D11DepthStencilState> m_depthStencilStateEqualAndDisableDepthWrite;
	
	ComPtr<ID3D11RasterizerState>	m_rasterState;
	ComPtr<ID3D11RasterizerState>	m_rasterStateNoCulling;
	ComPtr<ID3D11RasterizerState>	m_rasterStateWireframe;

	ComPtr<ID3D11BlendState>		m_blendStatepOpaque;
	ComPtr<ID3D11BlendState>		m_blendStatepDepthOnlyAlphaTest;
	ComPtr<ID3D11BlendState>		m_blendStateDepthOnlyAlphaToCoverage;
	ComPtr<ID3D11BlendState>		m_blendStateAlphaBlending;
	ComPtr<ID3D11BlendState>		m_blendOn;
	ComPtr<ID3D11BlendState>		m_blendOff;

	ComPtr<ID3D11BlendState>		m_blendStateParticle;
	
	ComPtr<ID3D11SamplerState>		m_linearSamplerState;		//Low settings
	ComPtr<ID3D11SamplerState>		m_anisotropicSamplerState;	//High settings
	ComPtr<ID3D11SamplerState>		m_pointSamplerState;
	ComPtr<ID3D11SamplerState>		m_cubemapSamplerState;

	ComPtr<ID3D11InputLayout>		m_defaultInputLayout;
	ComPtr<ID3D11InputLayout>		m_animationInputLayout;
	ComPtr<ID3D11InputLayout>		m_skyboxInputLayout;
	ComPtr<ID3D11InputLayout>		m_ParticleInputLayout;

	Shaders::VertexShader			m_defaultVertexShader;
	Shaders::VertexShader			m_depthPassVertexShader;
	Shaders::VertexShader			m_animationVertexShader;
	Shaders::VertexShader			m_skyboxVertexShader;
	Shaders::VertexShader			m_ParticleVertexShader;

	Shaders::PixelShader			m_depthPassPixelShader;
	Shaders::PixelShader			m_defaultPixelShader;
	Shaders::PixelShader			m_debugPixelShader;

	Shaders::ComputeShader			m_computeFrustums;
	Shaders::ComputeShader			m_lightCulling;

	Shaders::PixelShader			m_skyboxPixelShader;
	Shaders::PixelShader			m_ParticlePixelShader;

	Shaders::ComputeShader			m_ParticleComputeShader;
	Shaders::GeometryShader			m_ParticleGeometryShader;
	
	Shaders::ComputeShader			m_blurComputeShader;
	Shaders::ComputeShader			m_dofComputeShader;
	
	D3D11_VIEWPORT					m_viewport;


	bool CreateStructuredBuffer(ComPtr<ID3D11Buffer>& buffer, void* data, unsigned int byteStride,
		unsigned int arraySize, ComPtr<ID3D11UnorderedAccessView>& uav);

	bool CreateStructuredBuffer(void* data, unsigned int byteStride, unsigned int arraySize, ResourceAccessView &rav);

	void SetCullBack(bool cullNone);


	//
	// Forward+ Resources.
	//
	const uint32_t AVERAGE_OVERLAPPING_LIGHTS_PER_TILE = 100u;
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

	UINT							m_windowWidth;
	UINT							m_windowHeight;
	
	// View space frustums for the grid cells used in ForwardPlus rendering.
	ComPtr<ID3D11Buffer>				m_gridFrustum;
	ComPtr<ID3D11ShaderResourceView>	m_gridFrustumSRV;
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
};
