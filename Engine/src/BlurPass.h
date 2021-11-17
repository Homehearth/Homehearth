#pragma once
#include "IRenderPass.h"

const double PI = 3.14159265359;
const UINT MAXWEIGHT = 8;
const UINT MAXRADIUS = MAXWEIGHT - 1;
const UINT MINRADIUS = 1;
const UINT BLURLEVELSIZE = 5;

class BlurPass : public IRenderPass
{
private:
	BlurSettings m_blurSettings;
	std::pair<UINT, float> m_blurLevels[BLURLEVELSIZE];
	BlurLevel m_currentBlur;

	ComPtr<ID3D11UnorderedAccessView>	m_backBufferReadView;
	ComPtr<ID3D11UnorderedAccessView>	m_backBufferView;
	ComPtr<ID3D11Texture2D>				m_backBufferRead;
	ComPtr<ID3D11Buffer>				m_settingsBuffer;
public:
	BlurPass() = default;
	virtual ~BlurPass() = default;

	bool Create(BlurLevel level);

	// Inherited via IRenderPass
	virtual void PreRender(Camera* pCam = nullptr, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
	virtual void Render(Scene* pScene) override;
	virtual void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

private:

	bool CreateUnorderedAccessView();
	bool CreateSettingsBuffer();
	void SetUpBlurLevels();
	void GenerateGuassFilter(BlurLevel level);
	void UpdateBlurSettings();
	void SwapBlurDirection();
};