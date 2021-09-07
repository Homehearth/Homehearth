#pragma once


class Renderer
{
public:
	Renderer();
	virtual ~Renderer() = default;
	
	void initialize(Window* pWindow);

	void clearScreen()
	{
		const FLOAT color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		D3D11Core::Get().DeviceContext()->ClearRenderTargetView(this->renderTargetView.Get(), color);
	}
	
	void beginFrame()
	{

	}
	
	void draw()
	{
		
	}
	
	void endFrame()
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

	bool createRenderTargetView();
	bool createDepthStencilTexture();
	bool createDepthStencilState();
	bool createDepthStencilView();
	bool createRasterizerStates();
	bool createSamplerStates();
	void setViewport();
};

