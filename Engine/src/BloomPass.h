#pragma once
#include "IRenderPass.h"
#include "BlurPass.h"

class BloomPass : public IRenderPass
{
private:

	BlurPass							m_blurPass;
	ComPtr<ID3D11Texture2D>				m_fullSize; // Window Size o7
	ComPtr<ID3D11Texture2D>				m_halfSize; // (Window Size / 2)
	ComPtr<ID3D11Texture2D>				m_quarterSize; // (Window Size / 4)
	ComPtr<ID3D11Texture2D>				m_smolSize; // (Window Size / 8)

	ComPtr<ID3D11UnorderedAccessView>	m_fullSizeView;
	ComPtr<ID3D11UnorderedAccessView>	m_halfSizeView;
	ComPtr<ID3D11UnorderedAccessView>	m_quarterSizeView;
	ComPtr<ID3D11UnorderedAccessView>	m_smolSizeView;

public:

	BloomPass();
	~BloomPass() = default;

	// Inherited via IRenderPass
	virtual void PreRender(Camera* pCam = nullptr, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

	virtual void Render(Scene* pScene) override;

	virtual void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

};