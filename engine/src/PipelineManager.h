#pragma once
#include "Shader.h"

/*
 * This class holds the D3D11-specific resources used in different passes.
 */
class PipelineManager
{
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
	ComPtr<ID3D11RenderTargetView>	  m_backBuffer;
	ComPtr<ID3D11RenderTargetView>	  m_renderTargetView;

	ComPtr<ID3D11Texture2D>			  m_depthStencilTexture;
	ComPtr<ID3D11DepthStencilView>	  m_depthStencilView;
	ComPtr<ID3D11ShaderResourceView>  m_depthBufferSRV;

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

	Shaders::PixelShader			m_defaultPixelShader;
	Shaders::PixelShader			m_debugPixelShader;
	Shaders::PixelShader			m_skyboxPixelShader;
	Shaders::PixelShader			m_ParticlePixelShader;

	Shaders::ComputeShader			m_ParticleComputeShader;
	Shaders::GeometryShader			m_ParticleGeometryShader;
	
	Shaders::ComputeShader			m_blurComputeShader;
	Shaders::ComputeShader			m_dofComputeShader;
	
	D3D11_VIEWPORT					m_viewport;

	UINT							m_windowWidth;
	UINT							m_windowHeight;
	
	// View space frustums for the grid cells used in ForwardPlus rendering.
	ComPtr<ID3D11Buffer>				m_gridFrustum;
	ComPtr<ID3D11ShaderResourceView>	m_gridFrustumSRV;


	// Nikkis stuff
	Shaders::VertexShader             m_textureEffectVertexShader;  // Nikki's dummy water effect vertex shader.
	Shaders::ComputeShader            m_textureEffectComputeShader; // Nikki's water effect compute shader.
	Shaders::PixelShader              m_textureEffectPixelShader;   // Nikki's water effect pixel shader.
	ComPtr<ID3D11Buffer>			  m_textureEffectConstantBuffer;
	ComPtr<ID3D11Buffer>              m_deltaTimeBuffer;

	ComPtr<ID3D11ShaderResourceView>  m_SRV_TextureEffectBlendMap;
	ComPtr<ID3D11ShaderResourceView>  m_SRV_TextureEffectWaterMap;
	ComPtr<ID3D11ShaderResourceView>  m_SRV_TextureEffectWaterFloorMap;
	ComPtr<ID3D11ShaderResourceView>  m_SRV_TextureEffectWaterEdgeMap;
	ComPtr<ID3D11ShaderResourceView>  m_SRV_TextureEffectWaterNormalMap;
	ComPtr<ID3D11UnorderedAccessView> m_UAV_TextureEffectWaterFloorMap;


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
	std::shared_ptr<RTexture> m_WaterBlendAlbedoMap;

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
};


