#include "EnginePCH.h"
#include "BlurPass.h"

bool BlurPass::Create(BlurLevel pLevel, BlurType pType)
{
	SetUpBlurLevels();

	if (!CreateSettingsBuffer())
	{
		LOG_ERROR("failed to create settings buffer");
		return false;
	}

	if (!CreateUnorderedAccessView())
	{
		LOG_ERROR("failed to create UnorderedAccessView");
		return false;
	}

	if (pLevel != BlurLevel::NOBLUR)
	{
		m_currentBlur = pLevel;
		m_blurType = pType;
		m_blurSettings.blurType = UINT(pType);
		if (pType == BlurType::BOX)
			GenerateBoxFilter(pLevel);
		else if(pType == BlurType::GUASSIAN)
			GenerateGuassFilter(pLevel);

		UpdateBlurSettings();
	}

	return true;
}

void BlurPass::PreRenderTexture(Camera* pCam, ID3D11DeviceContext* pDeviceContext, ID3D11Texture2D* pTexture)
{
	if (m_currentBlur != BlurLevel::NOBLUR)
	{
		ID3D11RenderTargetView* nullRTV = nullptr;
		DC->OMSetRenderTargets(1, &nullRTV, nullptr);
		DC->CSSetShader(PM->m_blurComputeShader.Get(), nullptr, 0);
		DC->CSSetConstantBuffers(11, 1, m_settingsBuffer.GetAddressOf());
		DC->CopyResource(m_backBufferRead.Get(), pTexture);				
		DC->CSSetUnorderedAccessViews(0, 1, m_backBufferReadView.GetAddressOf(), nullptr);
		DC->CSSetUnorderedAccessViews(1, 1, m_backBufferView.GetAddressOf(), nullptr);
	}
}

void BlurPass::BlurTexture(ID3D11UnorderedAccessView* viewIn, ID3D11UnorderedAccessView* viewOut, ID3D11DeviceContext* pDeviceContext)
{
	if (m_currentBlur != BlurLevel::NOBLUR)
	{
		ID3D11RenderTargetView* nullRTV = nullptr;
		DC->OMSetRenderTargets(1, &nullRTV, nullptr);
		DC->CSSetShader(PM->m_blurComputeShader.Get(), nullptr, 0);
		DC->CSSetConstantBuffers(11, 1, m_settingsBuffer.GetAddressOf());

		DC->CSSetUnorderedAccessViews(0, 1, &viewIn, nullptr);
		DC->CSSetUnorderedAccessViews(1, 1, &viewOut, nullptr);

		if (m_blurType == BlurType::BOX)
		{
			DC->Dispatch(PM->m_windowWidth / 8, PM->m_windowHeight / 8, 1);
		}

		else if (m_blurType == BlurType::GUASSIAN)
		{
			DC->Dispatch(PM->m_windowWidth / 8, PM->m_windowHeight / 8, 1);
			SwapBlurDirection();
			DC->Dispatch(PM->m_windowWidth / 8, PM->m_windowHeight / 8, 1);
			SwapBlurDirection();
		}

		ID3D11UnorderedAccessView* nullUAV = nullptr;
		DC->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
		DC->CSSetUnorderedAccessViews(1, 1, &nullUAV, nullptr);

		ID3D11ComputeShader* nullCS = nullptr;
		DC->CSSetShader(nullCS, nullptr, 0);
	}
}

void BlurPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
	if (m_currentBlur != BlurLevel::NOBLUR)
	{
		ID3D11RenderTargetView* nullRTV = nullptr;
		DC->OMSetRenderTargets(1, &nullRTV, nullptr);
		DC->CSSetShader(PM->m_blurComputeShader.Get(), nullptr, 0);
		DC->CSSetConstantBuffers(11, 1, m_settingsBuffer.GetAddressOf());
		
		ID3D11Texture2D* backBuff = nullptr;
		if (FAILED(D3D11Core::Get().SwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuff))))
			return;
		DC->CopyResource(m_backBufferRead.Get(), backBuff);
		backBuff->Release();		

		DC->CSSetUnorderedAccessViews(0, 1, m_backBufferReadView.GetAddressOf(), nullptr);
		DC->CSSetUnorderedAccessViews(1, 1, m_backBufferView.GetAddressOf(), nullptr);
	}
}

void BlurPass::Render(Scene* pScene)
{
	if (m_blurType == BlurType::BOX)
	{
		D3D11Core::Get().DeviceContext()->Dispatch(PM->m_windowWidth / 8, PM->m_windowHeight / 8, 1);
	}

	else if (m_blurType == BlurType::GUASSIAN)
	{
		D3D11Core::Get().DeviceContext()->Dispatch(PM->m_windowWidth / 8, PM->m_windowHeight / 8, 1);
		SwapBlurDirection();
		D3D11Core::Get().DeviceContext()->Dispatch(PM->m_windowWidth / 8, PM->m_windowHeight / 8, 1);
		SwapBlurDirection();
	}
}

void BlurPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	DC->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
	DC->CSSetUnorderedAccessViews(1, 1, &nullUAV, nullptr);

	ID3D11ComputeShader* nullCS = nullptr;
	DC->CSSetShader(nullCS, nullptr, 0);
}

bool BlurPass::CreateUnorderedAccessView()
{
	ID3D11Texture2D* backBuff = nullptr;
	HRESULT hr = D3D11Core::Get().SwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuff));
	if (FAILED(hr))
	{
		backBuff->Release();
		return false;
	}

	D3D11_TEXTURE2D_DESC texDesc = {};
	backBuff->GetDesc(&texDesc);
	D3D11Core::Get().Device()->CreateTexture2D(&texDesc, nullptr, m_backBufferRead.GetAddressOf());
	D3D11Core::Get().DeviceContext()->CopyResource(m_backBufferRead.Get(), backBuff);
	hr = D3D11Core::Get().Device()->CreateUnorderedAccessView(backBuff, nullptr, m_backBufferView.GetAddressOf());
	if (FAILED(hr))
	{
		backBuff->Release();
		return false;
	}

	hr = D3D11Core::Get().Device()->CreateUnorderedAccessView(m_backBufferRead.Get(), nullptr, m_backBufferReadView.GetAddressOf());

	backBuff->Release();
	return !FAILED(hr);
}

bool BlurPass::CreateSettingsBuffer()
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = sizeof(BlurSettings);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	m_blurSettings.useVerticalBlur = true;
	m_blurSettings.blurRadius = 0;

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = &m_blurSettings;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&desc, &data, m_settingsBuffer.GetAddressOf());
	return !FAILED(hr);
}

void BlurPass::SetUpBlurLevels()
{
	m_blurLevels[(UINT)BlurLevel::SUPERLOW] = std::pair<UINT, float>(1, 0.5f);
	m_blurLevels[(UINT)BlurLevel::LOW] = std::pair<UINT, float>(2, 2.0f);
	m_blurLevels[(UINT)BlurLevel::MEDIUM] = std::pair<UINT, float>(3, 5.0f);
	m_blurLevels[(UINT)BlurLevel::HIGH] = std::pair<UINT, float>(3, 10.0f);
	m_blurLevels[(UINT)BlurLevel::SUPERHIGH] = std::pair<UINT, float>(4, 10.0f);
}

void BlurPass::GenerateGuassFilter(BlurLevel pLevel)
{
	UINT index = UINT(pLevel);

	if (index < BLURLEVELSIZE)
	{
		UINT radius = m_blurLevels[index].first;
		float sigma = m_blurLevels[index].second;

		//Only acceptable to have radiuses in the range of min and max
		if (radius > MAXRADIUS)
			radius = MAXRADIUS;
		if (radius < MINRADIUS)
			radius = MINRADIUS;

		m_blurSettings.blurRadius = radius;

		//If not sigma is picked we calculate it to fit with the curve
		if (sigma == 0)
			sigma = float(radius / 2.0f);

		for (int x = 0; x <= (int)radius; x++)
		{
			float weight = (float)(1.0f / sqrt(2 * PI * pow(sigma, 2)) * exp(-pow(x, 2) / (2 * pow(sigma, 2)))); //Gaussian equation 2 dimesional
			m_blurSettings.weights[x] = weight;
		}

		//Calculate the total sum
		float total = 0.0f;
		for (UINT i = 1; i <= radius; i++)
		{
			total += m_blurSettings.weights[i];
		}
		total *= 2;
		total += m_blurSettings.weights[0];

		if (total >= 0)
		{
			for (UINT i = 0; i <= radius; i++)
			{
				m_blurSettings.weights[i] /= total;
			}
		}
	}
}

void BlurPass::GenerateBoxFilter(BlurLevel pLevel)
{
	UINT index = UINT(pLevel);
	if (index < BLURLEVELSIZE)
	{
		UINT radius = m_blurLevels[index].first;
		//Only acceptable to have radiuses in the range of min and max
		if (radius > MAXRADIUS)
			radius = MAXRADIUS;
		if (radius < MINRADIUS)
			radius = MINRADIUS;

		m_blurSettings.blurRadius = radius * 2;
	}
}

void BlurPass::UpdateBlurSettings()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	D3D11Core::Get().DeviceContext()->Map(m_settingsBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &m_blurSettings, sizeof(BlurSettings));
	D3D11Core::Get().DeviceContext()->Unmap(m_settingsBuffer.Get(), 0);
}

void BlurPass::SwapBlurDirection()
{
	m_blurSettings.useVerticalBlur = !m_blurSettings.useVerticalBlur;
	UpdateBlurSettings();
}

ID3D11Texture2D* BlurPass::GetBackBuffer()
{
	return m_backBufferRead.Get();
}
