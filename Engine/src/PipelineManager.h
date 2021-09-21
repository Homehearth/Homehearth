#pragma once

// todo: move this somewhere else.
struct BasicModelMatrix
{
	sm::Matrix worldMatrix;
};


class PipelineManager
{
private:
	Window*							m_window;
	D3D11Core*						m_d3d11;
	
public:
	PipelineManager();
	virtual ~PipelineManager() = default;

	// Initialize PipelineManager.
	void Initialize(Window * pWindow);

	// PUBLIC DATA.
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
	
	ComPtr<ID3D11Buffer>			m_defaultModelConstantBuffer;
	ComPtr<ID3D11Buffer>			m_defaultViewConstantBuffer; // TODO: maybe put in Camera class or update from Camera class
	
	ComPtr<ID3D11VertexShader>		m_defaultVertexShader;
	ComPtr<ID3D11PixelShader>		m_defaultPixelShader;

	D3D11_VIEWPORT					m_viewport;

	
	std::string shaderByteCode;

private:
	// INITIALIZE METHODS.
	bool CreateRenderTargetView();
	bool CreateDepthStencilTexture();
	bool CreateDepthStencilState();
	bool CreateDepthStencilView();
	bool CreateRasterizerStates();
	bool CreateSamplerStates();

	bool CreateDefaultInputLayout();
	bool CreateDefaultConstantBuffer();
	bool CreateDefaultShaders();

	void SetViewport();
};


