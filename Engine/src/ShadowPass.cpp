#include "EnginePCH.h"
#include "ShadowPass.h"

//ID3D11DepthStencilView*& ShadowPass::EmplaceInMap(const unsigned int& index)
//{
//	// TODO: insert return statement here
//}

ShadowPass::ShadowPass()
{
}

ShadowPass::~ShadowPass()
{
}

void ShadowPass::CreateShadow(const comp::Light& light)
{
	ShadowSection shadow;
	shadow.light = light;
	//shadow.shadowDepth = EmplaceInMap(light.index);
	m_shadowMap.amount++;
}

void ShadowPass::SetupMap()
{
	if (m_shadowMap.amount <= 0)
		return;

	if (m_shadowMap.shadowTexture)
	{
		m_shadowMap.shadowTexture->Release();
		m_shadowMap.shadowTexture = nullptr;
	}
	if (m_shadowMap.shadowView)
	{
		m_shadowMap.shadowView->Release();
		m_shadowMap.shadowView = nullptr;
	}

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = SHADOW_SIZE;
	texDesc.Height = SHADOW_SIZE;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = (UINT)m_shadowMap.amount;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	HRESULT hr = D3D11Core::Get().Device()->CreateTexture2D(&texDesc, NULL, &m_shadowMap.shadowTexture);
	if (FAILED(hr))
	{
		LOG_WARNING("Failed setting up shadows, No shadows will be drawn.");
	}
}

void ShadowPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{

}

void ShadowPass::Render(Scene* pScene)
{

}

void ShadowPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{

}
