#pragma once
#include "IRenderPass.h"
#include "BlurPass.h"

class DOFPass : public IRenderPass
{
private:
	BlurPass							m_blurPass;
	ComPtr<ID3D11UnorderedAccessView>	m_inFocusView;
	ComPtr<ID3D11UnorderedAccessView>	m_outOfFocusView;
	ComPtr<ID3D11UnorderedAccessView>	m_depthView;
	ComPtr<ID3D11UnorderedAccessView>	m_outputView;
	ComPtr<ID3D11Texture2D>				m_inFocusTexture;
	ComPtr<ID3D11Texture2D>				m_outOfFocusTexture;
	ComPtr<ID3D11Texture2D>				m_depthTexture;
	ComPtr<ID3D11Texture2D>				m_outputTexture;
	ComPtr<ID3D11Buffer>				m_constBuff;

	struct inverseMatrix
	{
		sm::Matrix inverseView;
		sm::Matrix inverseProjection;
	}m_matrices;
public:
	DOFPass() = default;
	virtual ~DOFPass() = default;

	bool Create();

	// Inherited via IRenderPass
	virtual void PreRender(Camera* pCam = nullptr, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

	virtual void Render(Scene* pScene) override;

	virtual void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

private:
	bool CreateUnorderedAccessView();
	bool CreateBuffer();
};