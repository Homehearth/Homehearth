#pragma once
#include "IRenderPass.h"
#include "BlurPass.h"

/*
	The render views are as following:
	FULL
	HALF
	QUARTER
	TINY
	SMOL
	SMOLLEST
*/

class BloomPass : public IRenderPass
{
private:

	enum class RenderVersion
	{
		FULL_TO_HALF,
		HALF_TO_QUARTER,
		QUARTER_TO_TINY,
		TINY_TO_SMOL,
		SMOL_TO_SMOLLEST,
		SMOLLEST_TO_SMOL,
		SMOL_TO_TINY,
		TINY_TO_QUARTER,
		QUARTER_TO_HALF,
		HALF_TO_FULL
	};

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
	ComPtr<ID3D11RenderTargetView>		m_blurredTarget;

	ComPtr<ID3D11Texture2D>				m_halfSize; // (Window Size / 2)
	ComPtr<ID3D11ShaderResourceView>	m_halfSizeView;
	ComPtr<ID3D11RenderTargetView>		m_halfSizeRenderTarget;

	ComPtr<ID3D11Texture2D>				m_quarterSize; // (Window Size / 4)
	ComPtr<ID3D11ShaderResourceView>	m_quarterSizeView;
	ComPtr<ID3D11RenderTargetView>		m_quarterSizeRenderTarget;

	ComPtr<ID3D11Texture2D>				m_tinySize; // (Window Size / 8)
	ComPtr<ID3D11ShaderResourceView>	m_tinySizeView;
	ComPtr<ID3D11RenderTargetView>		m_tinySizeRenderTarget;

	ComPtr<ID3D11Texture2D>				m_smolSize; // (Window Size / 16)
	ComPtr<ID3D11ShaderResourceView>	m_smolSizeView;
	ComPtr<ID3D11RenderTargetView>		m_smolSizeRenderTarget;

	ComPtr<ID3D11Texture2D>				m_smollestSize; // (Window Size / 32)
	ComPtr<ID3D11ShaderResourceView>	m_smollestSizeView;
	ComPtr<ID3D11RenderTargetView>		m_smollestSizeRenderTarget;

	ID3D11Buffer*						m_screenSpaceQuad;
	ID3D11Buffer*						m_indexBuffer;
	dx::ConstantBuffer<sm::Vector4>		m_samplingInfoBuffer;
	ComPtr<ID3D11InputLayout>			m_inputLayout;

	void Unlink();
	void Draw(const RenderVersion& drawType);
	void AdditiveBlend();

public:

	BloomPass();
	~BloomPass();

	void Setup();

	// Inherited via IRenderPass
	virtual void PreRender(Camera* pCam = nullptr, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

	virtual void Render(Scene* pScene) override;

	virtual void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

};