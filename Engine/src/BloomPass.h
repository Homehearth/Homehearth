#pragma once
#include "IRenderPass.h"
#include "BlurPass.h"



class BloomPass : public IRenderPass
{
private:

	struct ScreenQuad
	{
		sm::Vector3 points;
		sm::Vector2 uv;
	};

	struct Quads
	{
		ScreenQuad quads[4];
	};

	struct Indices
	{
		UINT index[6];
	};

	struct Info
	{
		sm::Vector4 samplingInfo;
	}m_info;

	BlurPass							m_blurPass;

	ComPtr<ID3D11Texture2D>				m_fullSize; // Window Size o7
	ComPtr<ID3D11UnorderedAccessView>	m_fullSizeView;
	ComPtr<ID3D11RenderTargetView>		m_fullSizeTarget;

	ComPtr<ID3D11Texture2D>				m_blurredTexture;
	ComPtr<ID3D11UnorderedAccessView>	m_blurredAccess;
	ComPtr<ID3D11ShaderResourceView>	m_blurredView;

	ComPtr<ID3D11Texture2D>				m_halfSize; // (Window Size / 2)
	ComPtr<ID3D11ShaderResourceView>	m_halfSizeView;
	ComPtr<ID3D11RenderTargetView>		m_halfSizeRenderTarget;

	ID3D11Buffer*						m_screenSpaceQuad;
	ID3D11Buffer*						m_indexBuffer;
	dx::ConstantBuffer<sm::Vector4>		m_samplingInfoBuffer;
	ComPtr<ID3D11InputLayout>			m_inputLayout;

	void Unlink();
	void Setdownsample();
	void Setupsample();
	void Draw();

public:

	BloomPass();
	~BloomPass();

	void Setup();

	// Inherited via IRenderPass
	virtual void PreRender(Camera* pCam = nullptr, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

	virtual void Render(Scene* pScene) override;

	virtual void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

};