#pragma once
#include "IRenderPass.h"
#include "BlurPass.h"

enum class DoFType : UINT
{
	DEFAULT = 0,
	ADAPTIVE = 1,
	VIGNETTE = 2
};

class DOFPass : public IRenderPass
{
private:
	BlurPass							m_blurPass;
	DoFType								m_currentType;
	ComPtr<ID3D11UnorderedAccessView>	m_inFocusView;
	ComPtr<ID3D11UnorderedAccessView>	m_outOfFocusView;
	ComPtr<ID3D11UnorderedAccessView>	m_depthView;
	ComPtr<ID3D11UnorderedAccessView>	m_outputView;
	ComPtr<ID3D11Texture2D>				m_inFocusTexture;
	ComPtr<ID3D11Texture2D>				m_outOfFocusTexture;
	ComPtr<ID3D11Texture2D>				m_depthTexture;
	ComPtr<ID3D11Texture2D>				m_outputTexture;
	ComPtr<ID3D11Buffer>				m_constBuff;

	struct DoFHelpStruct
	{
		sm::Matrix		inverseView;
		sm::Matrix		inverseProjection;
		UINT			dofType;
		sm::Vector3		padding;
		sm::Vector4		playerPosView;
	}m_dofHelp;

public:
	DOFPass() = default;
	virtual ~DOFPass() = default;

	bool Create(const DoFType& pType);

	void SetDoFType(const DoFType& pType);

	// Inherited via IRenderPass
	virtual void PreRender(Camera* pCam = nullptr, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

	virtual void Render(Scene* pScene) override;

	virtual void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

private:
	bool CreateUnorderedAccessView();
	bool CreateBuffer();
};