#pragma once
#include "Shader.h"
#include "Camera.h"

/*
 * This class holds the D3D11-specific resources used in different passes.
 */
class PipelineManager
{
private:
	Window*							m_window;
	D3D11Core*						m_d3d11;
	
public:
	PipelineManager();
	virtual ~PipelineManager() = default;

	// Initialize PipelineManager.
	void Initialize(Window* pWindow);

	
	// PUBLIC AVAILABLE DATA.
	ComPtr<ID3D11RenderTargetView>	m_renderTargetView;

	ComPtr<ID3D11Texture2D>			m_depthStencilTexture;
	
	ComPtr<ID3D11DepthStencilView>	m_depthStencilView;
	ComPtr<ID3D11ShaderResourceView>m_depthStencilSRV;
	
	ComPtr<ID3D11DepthStencilState>	m_depthStencilStateLess;
	ComPtr<ID3D11DepthStencilState> m_depthStencilStateGreater;
	ComPtr<ID3D11DepthStencilState> m_depthStencilStateEqualAndDisableDepthWrite;

	ComPtr<ID3D11RasterizerState>	m_rasterState;
	ComPtr<ID3D11RasterizerState>	m_rasterStateNoCulling;
	ComPtr<ID3D11RasterizerState>	m_rasterStateWireframe;

	ComPtr<ID3D11BlendState>		m_blendStatepOpaque;
	ComPtr<ID3D11BlendState>		m_blendStatepDepthOnlyAlphaTest;
	ComPtr<ID3D11BlendState>		m_blendStateDepthOnlyAlphaToCoverage;;
	
	ComPtr<ID3D11SamplerState>		m_linearSamplerState;
	ComPtr<ID3D11SamplerState>		m_pointSamplerState;

	ComPtr<ID3D11InputLayout>		m_defaultInputLayout;
	ComPtr<ID3D11InputLayout>		m_positionOnlyInputLayout;
	
	ComPtr<ID3D11Buffer>			m_defaultModelConstantBuffer;	// TODO: maybe put in Camera class or update from Camera class
	
	Shaders::VertexShader			m_defaultVertexShader;
	Shaders::VertexShader			m_positionOnlyVertexShader;

	Shaders::PixelShader			m_defaultPixelShader;

	D3D11_VIEWPORT					m_viewport;

	

private:
	// INITIALIZE METHODS.
	
	bool CreateRenderTargetView();
	bool CreateDepthStencilTexture();
	bool CreateDepthStencilStates();
	bool CreateDepthStencilView();
	bool CreateRasterizerStates();
	bool CreateSamplerStates();
	bool CreateBlendStates();
	bool CreateShaders();
	bool CreateInputLayouts();	
	void SetViewport();

	bool CreateDefaultConstantBuffer();	// TODO: maybe put in Camera class or update from Camera class
};


