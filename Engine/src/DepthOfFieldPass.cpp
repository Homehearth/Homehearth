#include "EnginePCH.h"
#include "DepthOfFieldPass.h"

bool DOFPass::Create(const DoFType& pType)
{
	m_currentType = pType;
	m_dofHelp.dofType = UINT(m_currentType);

	if (!CreateUnorderedAccessView())
	{
		LOG_ERROR("Failed to create UnorderedAccessViews");
		return false;
	}

	if (!CreateBuffer())
	{
		LOG_ERROR("Failed to create Buffer");
		return false;
	}
	
	if (!m_blurPass.Create(BlurLevel::LOW, BlurType::BOX))
		return false;

	m_blurPass.Initialize(D3D11Core::Get().DeviceContext(), PM);

	return true;
}

void DOFPass::SetDoFType(const DoFType& pType)
{
	m_currentType = pType;
}

void DOFPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
	if (m_currentType != DoFType::DEFAULT)
	{
		ID3D11Texture2D* backBuffInFocus = nullptr;
		if (FAILED(D3D11Core::Get().SwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffInFocus))))
			return;
		DC->CopyResource(m_inFocusTexture.Get(), backBuffInFocus);
		DC->CopyResource(m_outputTexture.Get(), backBuffInFocus);
		backBuffInFocus->Release();

		m_blurPass.PreRender(pCam, DC);
		m_blurPass.Render(nullptr);

		ID3D11Texture2D* backBuffOutOfFocus = nullptr;
		if (FAILED(D3D11Core::Get().SwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffOutOfFocus))))
			return;
		DC->CopyResource(m_outOfFocusTexture.Get(), backBuffOutOfFocus);
		backBuffOutOfFocus->Release();

		ID3D11RenderTargetView* nullRTV = nullptr;
		DC->OMSetRenderTargets(1, &nullRTV, nullptr);
		DC->CSSetShader(PM->m_dofComputeShader.Get(), nullptr, 0);

		m_dofHelp.view = pCam->GetView();
		m_dofHelp.inverseProjection = pCam->GetProjection().Invert();
		m_dofHelp.dofType = UINT(m_currentType);


		DC->CSSetUnorderedAccessViews(2, 1, m_inFocusView.GetAddressOf(), nullptr);
		DC->CSSetUnorderedAccessViews(3, 1, m_outOfFocusView.GetAddressOf(), nullptr);
		DC->CSSetShaderResources(0, 1, PM->m_depthBufferSRV.GetAddressOf());
		DC->CSSetUnorderedAccessViews(4, 1, m_outputView.GetAddressOf(), nullptr);
		DC->CSSetConstantBuffers(12, 1, m_constBuff.GetAddressOf());
	}
}

void DOFPass::Render(Scene* pScene)
{
	if (m_currentType != DoFType::DEFAULT)
	{
		//pScene->ForEachComponent<comp::Player, comp::Network>([&](Entity& playerEntity, comp::Player& player, comp::Network& network)
		//	{
		//		if (*pScene->m_localPIDRef == network.id && network.id != UINT32_MAX)
		//		{
		//			sm::Vector3 pos = playerEntity.GetComponent<comp::Transform>()->position;
		//			m_dofHelp.playerPosView = sm::Vector4::Transform(sm::Vector4(pos.x, pos.y + 5.f, pos.z, 1.0f),
		//				pScene->GetCurrentCamera()->GetView());
		//		}
		//	});

		D3D11Core::Get().DeviceContext()->UpdateSubresource(m_constBuff.Get(), 0, nullptr, &m_dofHelp, 0, 0);
		D3D11Core::Get().DeviceContext()->Dispatch(PM->m_windowWidth / 8, PM->m_windowHeight / 8, 1);
		ID3D11Texture2D* outText = nullptr;
		if (FAILED(D3D11Core::Get().SwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&outText))))
			return;
		D3D11Core::Get().DeviceContext()->CopyResource(outText, m_outputTexture.Get());

		outText->Release();
	}
}

void DOFPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
	if (m_currentType != DoFType::DEFAULT)
	{
		m_blurPass.PostRender(DC);
		ID3D11UnorderedAccessView* nullUAV = nullptr;
		ID3D11ShaderResourceView* nullSRV = nullptr;
		DC->CSSetUnorderedAccessViews(2, 1, &nullUAV, nullptr);
		DC->CSSetUnorderedAccessViews(3, 1, &nullUAV, nullptr);
		DC->CSSetShaderResources(0, 1, &nullSRV);
		DC->CSSetUnorderedAccessViews(4, 1, &nullUAV, nullptr);

		ID3D11ComputeShader* nullCS = nullptr;
		DC->CSSetShader(nullCS, nullptr, 0);
	}
}

bool DOFPass::CreateUnorderedAccessView()
{
	ID3D11Texture2D* backBuff = nullptr;
	HRESULT hr = D3D11Core::Get().SwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuff));
	if(FAILED(hr))
		return false;

	D3D11_TEXTURE2D_DESC texDesc = {};
	backBuff->GetDesc(&texDesc);
	D3D11Core::Get().Device()->CreateTexture2D(&texDesc, nullptr, m_inFocusTexture.GetAddressOf());
	D3D11Core::Get().Device()->CreateTexture2D(&texDesc, nullptr, m_outOfFocusTexture.GetAddressOf());
	D3D11Core::Get().Device()->CreateTexture2D(&texDesc, nullptr, m_depthTexture.GetAddressOf());
	D3D11Core::Get().Device()->CreateTexture2D(&texDesc, nullptr, m_outputTexture.GetAddressOf());
	D3D11Core::Get().DeviceContext()->CopyResource(m_inFocusTexture.Get(), backBuff);
	backBuff->Release();


	hr = D3D11Core::Get().Device()->CreateUnorderedAccessView(m_inFocusTexture.Get(), nullptr, m_inFocusView.GetAddressOf());
	if (FAILED(hr))
		return false;

	D3D11Core::Get().DeviceContext()->CopyResource(m_outOfFocusTexture.Get(), m_inFocusTexture.Get());
	hr = D3D11Core::Get().Device()->CreateUnorderedAccessView(m_outOfFocusTexture.Get(), nullptr, m_outOfFocusView.GetAddressOf());

	if (FAILED(hr))
		return false;

	D3D11Core::Get().DeviceContext()->CopyResource(m_outputTexture.Get(), m_inFocusTexture.Get());
	hr = D3D11Core::Get().Device()->CreateUnorderedAccessView(m_outputTexture.Get(), nullptr, m_outputView.GetAddressOf());

	if (FAILED(hr))
		return false;

	return true;
}

bool DOFPass::CreateBuffer()
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeof(DoFHelpStruct);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.MiscFlags = 0;

	HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&desc, nullptr, m_constBuff.GetAddressOf());

	return !FAILED(hr);
}
