#pragma once


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
	}
	
	void BeginFrame()
	{
		this->ClearScreen();
		D3D11Core::Get().DeviceContext()->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), nullptr);
	}
	
	void EndFrame()
	{
		
	}

private:
	Window* pWindow;
	bool isInitialized;
	
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

	bool CreateRenderTargetView();
	bool CreateDepthStencilTexture();
	bool CreateDepthStencilState();
	bool CreateDepthStencilView();
	bool CreateRasterizerStates();
	bool CreateSamplerStates();
	void SetViewport();
};

