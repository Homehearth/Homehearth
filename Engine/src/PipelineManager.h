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
	D3D11_VIEWPORT					m_viewport;
public:
	PipelineManager();
	virtual ~PipelineManager() = default;

	// Initialize PipelineManager.
	void Initialize(Window * pWindow);

	
	// PUBLIC AVAILABLE DATA.
	
	ComPtr<ID3D11RenderTargetView>	m_renderTargetView;

	ComPtr<ID3D11Texture2D>			m_depthStencilTexture;
	
	ComPtr<ID3D11DepthStencilView>	m_depthStencilView;
	ComPtr<ID3D11DepthStencilState>	m_depthStencilState;

	ComPtr<ID3D11RasterizerState>	m_rasterizerState;
	ComPtr<ID3D11RasterizerState>	m_rasterStateNoCulling;
	ComPtr<ID3D11RasterizerState>	m_rasterStateWireframe;

	ComPtr<ID3D11SamplerState>		m_linearSamplerState;
	ComPtr<ID3D11SamplerState>		m_pointSamplerState;

	ComPtr<ID3D11InputLayout>		m_defaultInputLayout;
	
	ComPtr<ID3D11Buffer>			m_defaultConstantBuffer;

	Shaders::VertexShader			m_defaultVertexShader;
	Shaders::PixelShader			m_defaultPixelShader;


private:
	// INITIALIZE METHODS.
	
	bool CreateRenderTargetView();
	bool CreateDepthStencilTexture();
	bool CreateDepthStencilState();
	bool CreateDepthStencilView();
	bool CreateRasterizerStates();
	bool CreateSamplerStates();
	bool CreateShaders();
	void SetViewport();

	bool CreateDefaultInputLayout();	// todo: create all input layouts.
	bool CreateDefaultConstantBuffer();	// todo: make use the HelperBuffer.h.
};


