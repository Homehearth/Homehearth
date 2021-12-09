#include "EnginePCH.h"
#include "BloomPass.h"

void BloomPass::Unlink()
{
	ID3D11ShaderResourceView* nullSRV = nullptr;
	D3D11Core::Get().DeviceContext()->PSSetShaderResources(1, 1, &nullSRV);
    ID3D11RenderTargetView* nullTarget = nullptr;
    D3D11Core::Get().DeviceContext()->OMSetRenderTargets(1, &nullTarget, nullptr);
    ID3D11UnorderedAccessView* nullAccess = nullptr;
    D3D11Core::Get().DeviceContext()->CSSetUnorderedAccessViews(0, 1, &nullAccess, nullptr);
    D3D11Core::Get().DeviceContext()->CSSetUnorderedAccessViews(1, 1, &nullAccess, nullptr);
    ID3D11RenderTargetView* nullRTV[8] = { nullptr };
    D3D11Core::Get().DeviceContext()->OMSetRenderTargets(8, nullRTV, nullptr);
}

void BloomPass::Draw(const RenderVersion& drawType)
{
    this->Unlink();
    switch (drawType)
    {
    case RenderVersion::FULL_TO_HALF:
    {
        m_info.samplingInfo = { 2.0f, 1.0f, 1.0f, 1.0f };
        m_samplingInfoBuffer.SetData(D3D11Core::Get().DeviceContext(), m_info.samplingInfo);
        D3D11Core::Get().DeviceContext()->PSSetShaderResources(1, 1, m_blurredView.GetAddressOf());
        D3D11Core::Get().DeviceContext()->OMSetRenderTargets(1, m_halfSizeRenderTarget.GetAddressOf(), nullptr);
        break;
    }
    case RenderVersion::HALF_TO_QUARTER:
    {
        m_info.samplingInfo = { 4.0f, 1.0f, 1.0f, 1.0f };
        m_samplingInfoBuffer.SetData(D3D11Core::Get().DeviceContext(), m_info.samplingInfo);
        D3D11Core::Get().DeviceContext()->PSSetShaderResources(1, 1, m_halfSizeView.GetAddressOf());
        D3D11Core::Get().DeviceContext()->OMSetRenderTargets(1, m_quarterBlurredSizeRenderTarget.GetAddressOf(), nullptr);
        break;
    }
    case RenderVersion::QUARTER_TO_TINY:
    {
        m_info.samplingInfo = { 8.0f, 1.0f, 1.0f, 1.0f };
        m_samplingInfoBuffer.SetData(D3D11Core::Get().DeviceContext(), m_info.samplingInfo);
        D3D11Core::Get().DeviceContext()->PSSetShaderResources(1, 1, m_quarterSizeView.GetAddressOf());
        D3D11Core::Get().DeviceContext()->OMSetRenderTargets(1, m_tinySizeRenderTarget.GetAddressOf(), nullptr);
        break;
    }
    case RenderVersion::TINY_TO_SMOL:
    {
        m_info.samplingInfo = { 16.0f, 1.0f, 1.0f, 1.0f };
        m_samplingInfoBuffer.SetData(D3D11Core::Get().DeviceContext(), m_info.samplingInfo);
        D3D11Core::Get().DeviceContext()->PSSetShaderResources(1, 1, m_tinySizeView.GetAddressOf());
        D3D11Core::Get().DeviceContext()->OMSetRenderTargets(1, m_smolSizeRenderTarget.GetAddressOf(), nullptr);
        break;
    }
    case RenderVersion::SMOL_TO_SMOLLEST:
    {
        m_info.samplingInfo = { 32.0f, 1.0f, 1.0f, 1.0f };
        m_samplingInfoBuffer.SetData(D3D11Core::Get().DeviceContext(), m_info.samplingInfo);
        D3D11Core::Get().DeviceContext()->PSSetShaderResources(1, 1, m_smolSizeView.GetAddressOf());
        D3D11Core::Get().DeviceContext()->OMSetRenderTargets(1, m_smollestSizeRenderTarget.GetAddressOf(), nullptr);
        break;
    }
    case RenderVersion::SMOLLEST_TO_SMOL:
    {
        m_info.samplingInfo = { 16.0f, 1.0f, 1.0f, 1.0f };
        m_samplingInfoBuffer.SetData(D3D11Core::Get().DeviceContext(), m_info.samplingInfo);
        D3D11Core::Get().DeviceContext()->PSSetShaderResources(1, 1, m_smollestSizeView.GetAddressOf());
        D3D11Core::Get().DeviceContext()->OMSetRenderTargets(1, m_smolSizeRenderTarget.GetAddressOf(), nullptr);
        break;
    }
    case RenderVersion::SMOL_TO_TINY:
    {
        m_info.samplingInfo = { 8.0f, 1.0f, 1.0f, 1.0f };
        m_samplingInfoBuffer.SetData(D3D11Core::Get().DeviceContext(), m_info.samplingInfo);
        D3D11Core::Get().DeviceContext()->PSSetShaderResources(1, 1, m_smolSizeView.GetAddressOf());
        D3D11Core::Get().DeviceContext()->OMSetRenderTargets(1, m_tinySizeRenderTarget.GetAddressOf(), nullptr);
        break;
    }
    case RenderVersion::TINY_TO_QUARTER:
    {
        m_info.samplingInfo = { 4.0f, 1.0f, 1.0f, 1.0f };
        m_samplingInfoBuffer.SetData(D3D11Core::Get().DeviceContext(), m_info.samplingInfo);
        D3D11Core::Get().DeviceContext()->PSSetShaderResources(1, 1, m_tinySizeView.GetAddressOf());
        D3D11Core::Get().DeviceContext()->OMSetRenderTargets(1, m_quarterSizeRenderTarget.GetAddressOf(), nullptr);
        break;
    }
    case RenderVersion::QUARTER_TO_HALF:
    {
        m_info.samplingInfo = { 2.0f, 1.0f, 1.0f, 1.0f };
        m_samplingInfoBuffer.SetData(D3D11Core::Get().DeviceContext(), m_info.samplingInfo);
        D3D11Core::Get().DeviceContext()->PSSetShaderResources(1, 1, m_quarterSizeView.GetAddressOf());
        D3D11Core::Get().DeviceContext()->OMSetRenderTargets(1, m_halfSizeRenderTarget.GetAddressOf(), nullptr);
        break;
    }
    case RenderVersion::HALF_TO_FULL:
    {
        m_info.samplingInfo = { 1.0f, 1.0f, 1.0f, 1.0f };
        m_samplingInfoBuffer.SetData(D3D11Core::Get().DeviceContext(), m_info.samplingInfo);
        D3D11Core::Get().DeviceContext()->PSSetShaderResources(1, 1, m_halfSizeView.GetAddressOf());
        D3D11Core::Get().DeviceContext()->OMSetRenderTargets(1, m_blurredTarget.GetAddressOf(), nullptr);
        break;
    }
    default:
    {
        break;
    }
    }
    D3D11Core::Get().DeviceContext()->RSSetViewports(1, &PM->m_viewport);
    D3D11Core::Get().DeviceContext()->VSSetShader(nullptr, nullptr, 0);
    D3D11Core::Get().DeviceContext()->PSSetShader(nullptr, nullptr, 0);
    D3D11Core::Get().DeviceContext()->VSSetShader(PM->m_bloomVertexShader.Get(), nullptr, 0);
    D3D11Core::Get().DeviceContext()->PSSetShader(PM->m_bloomPixelShader.Get(), nullptr, 0);
    D3D11Core::Get().DeviceContext()->IASetInputLayout(m_inputLayout.Get());
    D3D11Core::Get().DeviceContext()->PSSetSamplers(1, 1, PM->m_linearSamplerState.GetAddressOf());

    UINT stride = sizeof(ScreenQuad);
    UINT offset = 0;

    ID3D11Buffer* buff = { m_samplingInfoBuffer.GetBuffer() };

    D3D11Core::Get().DeviceContext()->PSSetConstantBuffers(4, 1, &buff);
    D3D11Core::Get().DeviceContext()->IASetVertexBuffers(0, 1, &m_screenSpaceQuad, &stride, &offset);
    D3D11Core::Get().DeviceContext()->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    D3D11Core::Get().DeviceContext()->DrawIndexed(6, 0, 0);
}

void BloomPass::AdditiveBlend()
{
    this->Unlink();
    D3D11Core::Get().DeviceContext()->CSSetUnorderedAccessViews(1, 1, PM->m_backBufferAccessView.GetAddressOf(), nullptr);
    D3D11Core::Get().DeviceContext()->CSSetUnorderedAccessViews(0, 1, m_blurredAccess.GetAddressOf(), nullptr);
    D3D11Core::Get().DeviceContext()->CSSetShader(PM->m_bloomComputeShader.Get(), nullptr, 0);
    D3D11Core::Get().DeviceContext()->Dispatch(PM->m_windowWidth / 8, PM->m_windowHeight / 8, 1);
    this->Unlink();
}

BloomPass::BloomPass()
{
    m_screenSpaceQuad = nullptr;
    m_indexBuffer = nullptr;
}

BloomPass::~BloomPass()
{
    if (m_screenSpaceQuad)
        m_screenSpaceQuad->Release();
    if (m_indexBuffer)
        m_indexBuffer->Release();
}

void BloomPass::Setup()
{
	m_blurPass.Initialize(D3D11Core::Get().DeviceContext(), PM);
	m_blurPass.Create(BlurLevel::MEDIUM, BlurType::BOX);
    m_samplingInfoBuffer.Create(D3D11Core::Get().Device());

    HRESULT hr = S_FALSE;

    // Create m_defaultInputLayout.
    std::string shaderByteCode = PM->m_bloomVertexShader.GetShaderByteCode();
    D3D11_INPUT_ELEMENT_DESC defaultVertexShaderDesc[] =
    {
        {"POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0,                0,                   D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    if (FAILED(D3D11Core::Get().Device()->CreateInputLayout(defaultVertexShaderDesc, ARRAYSIZE(defaultVertexShaderDesc), shaderByteCode.c_str(), shaderByteCode.length(), m_inputLayout.GetAddressOf())))
    {
        LOG_WARNING("failed creating bloom input layout");
        return;
    }

    ScreenQuad verts[4] =
    {
        // Pos                UV
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // Top Left
        {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // Top Right
        {{1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}}, // Bottom Right
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}} // Bottom Left
    };

    UINT index[6] =
    {
        1,2,3,
        0,1,3
    };

    Indices indices;
    indices.index[0] = index[0];
    indices.index[1] = index[1];
    indices.index[2] = index[2];
    indices.index[3] = index[3];
    indices.index[4] = index[4];
    indices.index[5] = index[5];

    Quads screenQuads;
    screenQuads.quads[0] = verts[0];
    screenQuads.quads[1] = verts[1];
    screenQuads.quads[2] = verts[2];
    screenQuads.quads[3] = verts[3];

    D3D11_BUFFER_DESC bDesc;
    bDesc.ByteWidth = sizeof(Quads);
    bDesc.Usage = D3D11_USAGE_DEFAULT;
    bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bDesc.CPUAccessFlags = 0;
    bDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = &screenQuads;

    D3D11Core::Get().Device()->CreateBuffer(&bDesc, &data, &m_screenSpaceQuad);

    D3D11_BUFFER_DESC bDesc1;
    bDesc1.ByteWidth = sizeof(Indices);
    bDesc1.Usage = D3D11_USAGE_DEFAULT;
    bDesc1.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bDesc1.CPUAccessFlags = 0;
    bDesc1.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA data1;
    data1.pSysMem = &indices;

    D3D11Core::Get().Device()->CreateBuffer(&bDesc1, &data1, &m_indexBuffer);

    ID3D11Texture2D* backBuff = nullptr;
    hr = D3D11Core::Get().SwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuff));
    if (FAILED(hr))
    {
        backBuff->Release();
        return;
    }

    D3D11_TEXTURE2D_DESC texDesc = {};
    backBuff->GetDesc(&texDesc);
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
    hr = D3D11Core::Get().Device()->CreateTexture2D(&texDesc, nullptr, m_fullSize.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateUnorderedAccessView(m_fullSize.Get(), NULL, m_fullSizeView.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateRenderTargetView(m_fullSize.Get(), NULL, m_fullSizeTarget.GetAddressOf());

    hr = D3D11Core::Get().Device()->CreateTexture2D(&texDesc, nullptr, m_blurredTexture.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateShaderResourceView(m_blurredTexture.Get(), NULL, m_blurredView.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateUnorderedAccessView(m_blurredTexture.Get(), NULL, m_blurredAccess.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateRenderTargetView(m_blurredTexture.Get(), NULL, m_blurredTarget.GetAddressOf());


    texDesc.Height /= 2;
    texDesc.Width /= 2;
    hr = D3D11Core::Get().Device()->CreateTexture2D(&texDesc, nullptr, m_halfSize.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateShaderResourceView(m_halfSize.Get(), NULL, m_halfSizeView.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateRenderTargetView(m_halfSize.Get(), NULL, m_halfSizeRenderTarget.GetAddressOf());
    backBuff->Release();

    texDesc.Height /= 2;
    texDesc.Width /= 2;
    hr = D3D11Core::Get().Device()->CreateTexture2D(&texDesc, nullptr, m_quarterSize.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateShaderResourceView(m_quarterSize.Get(), NULL, m_quarterSizeView.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateRenderTargetView(m_quarterSize.Get(), NULL, m_quarterSizeRenderTarget.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateUnorderedAccessView(m_quarterSize.Get(), NULL, m_quarterSizeAccess.GetAddressOf());

    hr = D3D11Core::Get().Device()->CreateTexture2D(&texDesc, nullptr, m_quarterBlurredSize.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateShaderResourceView(m_quarterBlurredSize.Get(), NULL, m_quarterBlurredSizeView.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateRenderTargetView(m_quarterBlurredSize.Get(), NULL, m_quarterBlurredSizeRenderTarget.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateUnorderedAccessView(m_quarterBlurredSize.Get(), NULL, m_quarterBlurredAccess.GetAddressOf());

    texDesc.Height /= 2;
    texDesc.Width /= 2;
    hr = D3D11Core::Get().Device()->CreateTexture2D(&texDesc, nullptr, m_tinySize.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateShaderResourceView(m_tinySize.Get(), NULL, m_tinySizeView.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateRenderTargetView(m_tinySize.Get(), NULL, m_tinySizeRenderTarget.GetAddressOf());

    texDesc.Height /= 2;
    texDesc.Width /= 2;
    hr = D3D11Core::Get().Device()->CreateTexture2D(&texDesc, nullptr, m_smolSize.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateShaderResourceView(m_smolSize.Get(), NULL, m_smolSizeView.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateRenderTargetView(m_smolSize.Get(), NULL, m_smolSizeRenderTarget.GetAddressOf());

    texDesc.Height /= 2;
    texDesc.Width /= 2;
    hr = D3D11Core::Get().Device()->CreateTexture2D(&texDesc, nullptr, m_smollestSize.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateShaderResourceView(m_smollestSize.Get(), NULL, m_smollestSizeView.GetAddressOf());
    hr = D3D11Core::Get().Device()->CreateRenderTargetView(m_smollestSize.Get(), NULL, m_smollestSizeRenderTarget.GetAddressOf());
}

void BloomPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{   
    //ID3D11Texture2D* backBuff = nullptr;
    //HRESULT hr = D3D11Core::Get().SwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuff));
    //if (FAILED(hr))
    //{
    //    backBuff->Release();
    //    return;
    //}
    //DC->CopyResource(m_blurredTexture.Get(), backBuff);
    //backBuff->Release();

    DC->CopyResource(m_blurredTexture.Get(), PM->m_bloomTexture.Get());

    //m_blurPass.BlurTexture(PM->m_bloomAccessView.GetAddressOf(), m_blurredAccess.GetAddressOf(), DC);
}

void BloomPass::Render(Scene* pScene)
{
    this->Draw(RenderVersion::FULL_TO_HALF);
    this->Draw(RenderVersion::HALF_TO_QUARTER);
    m_blurPass.BlurTexture(sm::Vector2(PM->m_windowWidth / 4, PM->m_windowHeight / 4), m_quarterBlurredAccess.GetAddressOf(), m_quarterSizeAccess.GetAddressOf());
    this->Draw(RenderVersion::QUARTER_TO_TINY);
    this->Draw(RenderVersion::TINY_TO_SMOL);
    this->Draw(RenderVersion::SMOL_TO_SMOLLEST);
    this->Draw(RenderVersion::SMOLLEST_TO_SMOL);
    this->Draw(RenderVersion::SMOL_TO_TINY);
    this->Draw(RenderVersion::TINY_TO_QUARTER);
    this->Draw(RenderVersion::QUARTER_TO_HALF);
    this->Draw(RenderVersion::HALF_TO_FULL);
    this->AdditiveBlend();
}

void BloomPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
}
