#pragma once

/*
	Used for default rendering, sets the object position 
	at origo.
*/
struct BasicModelMatrix
{
	sm::Matrix worldMatrix;
};

class Renderer
{
public:
	Renderer();
	virtual ~Renderer() = default;
	
	void Initialize(Window* pWindow);

	void ClearScreen()
	{
		const FLOAT color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		D3D11Core::Get().DeviceContext()->ClearRenderTargetView(this->renderTargetView.Get(), color);
		D3D11Core::Get().DeviceContext()->ClearDepthStencilView(this->depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}
	
	void BeginFrame()
	{
		this->ClearScreen();
		D3D11Core::Get().DeviceContext()->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), nullptr);
	}
	
	void EndFrame()
	{
		
	}
	void SetPipelineState();
private:
	Window* m_window;
	D3D11Core * m_d3d11;
		
	ComPtr<ID3D11RenderTargetView>		renderTargetView;
	
	ComPtr<ID3D11Texture2D>				depthStencilTexture;
	ComPtr<ID3D11DepthStencilView>		depthStencilView;
	ComPtr<ID3D11DepthStencilState>		depthStencilState;
	
	ComPtr<ID3D11RasterizerState>		rasterizerState;
	ComPtr<ID3D11RasterizerState>		rasterStateNoCulling;
	ComPtr<ID3D11RasterizerState>		rasterStateWireframe;
	
	ComPtr<ID3D11SamplerState>			linearSamplerState;
	ComPtr<ID3D11SamplerState>			pointSamplerState;
	
	D3D11_VIEWPORT						viewport;


	// Initialize Methods.
	bool CreateRenderTargetView();
	bool CreateDepthStencilTexture();
	bool CreateDepthStencilState();
	bool CreateDepthStencilView();
	bool CreateRasterizerStates();
	bool CreateSamplerStates();
	void SetViewport();

	
	// Required Structs.
	enum class RenderingTechnique
	{
		FORWARD,
		FORWARD_PLUS
	};
	
	// Forward-Rendering Technique.
	struct fullscreen_t {};
	
	


	// Temporary to not get any errors.
	using ETopology = int;
	using RenderTargetIDs = int;
	using DepthTargetID = int;
	using RasterizerStateID = int;
	using DepthStencilStateID = int;
	using BlendStateID = int;
	using ShaderID = int;
	using BufferID = int;
	using ConstantBufferID = int;
	using Viewport = int;
	
	// PipelineState holds the settings for a draw call.
	struct PipelineState
	{
		ETopology				topology;
		RenderTargetIDs			renderTargets;
		DepthTargetID			depthTargets;
		RasterizerStateID		rasterizerState;
		DepthStencilStateID		depthStencilState;
		BlendStateID			blendState;
		ShaderID				shader;
		BufferID				vertexBuffer;
		BufferID				indexBuffer;
		ConstantBufferID		constantBuffer;
		Viewport				viewPort;
	};

	std::vector<PipelineState> pipelineStates;

private:

	ComPtr<ID3D11InputLayout> m_defaultLayout;
	ComPtr<ID3D11Buffer> m_defaultcBuffer;
	ComPtr<ID3D11VertexShader> m_defaultVertexShader;
	ComPtr<ID3D11PixelShader> m_defaultPixelShader;
	std::string shaderByteCode;

	bool CreateDefaultLayout();
	bool CreateDefaultcBuffer();
	bool CreateDefaultShaders();
};

