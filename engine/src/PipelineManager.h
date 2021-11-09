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
	ComPtr<ID3D11RenderTargetView>	m_backBuffer;
	ComPtr<ID3D11RenderTargetView>	  m_renderTargetView;
	ComPtr<ID3D11RenderTargetView>    m_RTV_TextureEffectDiffuseMap;

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
	ComPtr<ID3D11Texture2D>			  m_depthStencilTexture;
	//ComPtr<ID3D11Texture2D>			  m_TextureEffectDiffuseMap; 
	//ComPtr<ID3D11Texture2D>			  m_TextureEffectDisplacementMap;

	ComPtr<ID3D11DepthStencilView>	  m_depthStencilView;

	//ComPtr<ID3D11ShaderResourceView>  m_SRV_TextureEffectDisplacementMap;
	ComPtr<ID3D11ShaderResourceView>  m_depthStencilSRV;

	ComPtr<ID3D11DepthStencilState>   m_depthStencilStateLess;
	ComPtr<ID3D11DepthStencilState>   m_depthStencilStateGreater;
	ComPtr<ID3D11DepthStencilState>   m_depthStencilStateEqualAndDisableDepthWrite;

	ComPtr<ID3D11RasterizerState>     m_rasterState;
	ComPtr<ID3D11RasterizerState>	  m_rasterStateNoCulling;
	ComPtr<ID3D11RasterizerState>	  m_rasterStateWireframe;

	ComPtr<ID3D11BlendState>		  m_blendStatepOpaque;
	ComPtr<ID3D11BlendState>		  m_blendStatepDepthOnlyAlphaTest;
	ComPtr<ID3D11BlendState>		  m_blendStateDepthOnlyAlphaToCoverage;;
	
	ComPtr<ID3D11SamplerState>		  m_linearSamplerState;
	ComPtr<ID3D11SamplerState>		  m_pointSamplerState;

	ComPtr<ID3D11InputLayout>		  m_defaultInputLayout;
	ComPtr<ID3D11InputLayout>		  m_positionOnlyInputLayout;
	
	ComPtr<ID3D11SamplerState>		m_linearSamplerState;		//Low settings
	ComPtr<ID3D11SamplerState>		m_anisotropicSamplerState;	//High settings
	ComPtr<ID3D11SamplerState>		m_pointSamplerState;
	ComPtr<ID3D11Buffer>			  m_textureEffectConstantBuffer; // Nikki's water effect constant buffer (Only has delta time) 

	ComPtr<ID3D11InputLayout>		m_defaultInputLayout;
	ComPtr<ID3D11InputLayout>		m_animationInputLayout;

	Shaders::VertexShader			m_defaultVertexShader;
	Shaders::VertexShader			m_depthPassVertexShader;
	Shaders::VertexShader			m_animationVertexShader;
	Shaders::VertexShader			  m_defaultVertexShader;
	Shaders::VertexShader			  m_positionOnlyVertexShader;

	Shaders::PixelShader			  m_defaultPixelShader;

	// Nikkis Texture Effect shaders.
	Shaders::VertexShader             m_textureEffectVertexShader;  // Nikki's dummy water effect vertex shader.
	Shaders::PixelShader              m_textureEffectPixelShader;   // Nikki's water effect pixel shader.
	Shaders::ComputeShader            m_textureEffectComputeShader; // Nikki's water effect compute shader.

	Shaders::PixelShader			m_defaultPixelShader;
	Shaders::PixelShader			m_debugPixelShader;
	
	D3D11_VIEWPORT					m_viewport;
	RTexture m_TextureEffectDiffuseMap;
	RTexture m_TextureEffectDisplacementMap;

	D3D11_VIEWPORT					  m_viewport;

	
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
	bool CreateRTextures();
	void SetViewport();

	bool CreateTextureEffectConstantBuffer();
};


