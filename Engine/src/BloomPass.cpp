#include "EnginePCH.h"
#include "BloomPass.h"

void BloomPass::Unlink()
{
	ID3D11ShaderResourceView* nullSRV = nullptr;
	D3D11Core::Get().DeviceContext()->PSSetShaderResources(0, 1, &nullSRV);
}

BloomPass::BloomPass()
{

}

void BloomPass::Setup()
{
	m_blurPass.Initialize(D3D11Core::Get().DeviceContext(), PM);
	m_blurPass.Create(BlurLevel::SUPERHIGH, BlurType::GUASSIAN);
	ID3D11Texture2D* backBuff = nullptr;
	HRESULT hr = D3D11Core::Get().SwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuff));
	if (FAILED(hr))
	{
		backBuff->Release();
		return;
	}
	D3D11_TEXTURE2D_DESC texDesc = {};
	backBuff->GetDesc(&texDesc);
	backBuff->Release();
	D3D11Core::Get().Device()->CreateTexture2D(&texDesc, nullptr, m_fullSize.GetAddressOf());

	// Create m_defaultInputLayout.
	std::string shaderByteCode = PM->m_bloomVertexShader.GetShaderByteCode();
	D3D11_INPUT_ELEMENT_DESC defaultVertexShaderDesc[] =
	{
		{"POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0,                0,                   D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	if (FAILED(D3D11Core::Get().Device()->CreateInputLayout(defaultVertexShaderDesc, ARRAYSIZE(defaultVertexShaderDesc), shaderByteCode.c_str(), shaderByteCode.length(), m_inputLayout.GetAddressOf())))
	{
		LOG_WARNING("failed creating m_defaultInputLayout.");
		return;
	}
}

void BloomPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{

}

void BloomPass::Render(Scene* pScene)
{

}

void BloomPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
}
