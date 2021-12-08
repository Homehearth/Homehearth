#include "EnginePCH.h"
#include "BloomPass.h"

void BloomPass::Unlink()
{
	ID3D11ShaderResourceView* nullSRV = nullptr;
	D3D11Core::Get().DeviceContext()->PSSetShaderResources(1, 1, &nullSRV);
    ID3D11RenderTargetView* nullTarget = nullptr;
    D3D11Core::Get().DeviceContext()->OMSetRenderTargets(1, &nullTarget, nullptr);
}

void BloomPass::Setdownsample()
{
    this->Unlink();
    m_info.samplingInfo = { 4.0f, 1.0f, 1.0f, 1.0f };
    m_samplingInfoBuffer.SetData(D3D11Core::Get().DeviceContext(), m_info.samplingInfo);
    D3D11Core::Get().DeviceContext()->PSSetShaderResources(1, 1, m_blurredView.GetAddressOf());
    D3D11Core::Get().DeviceContext()->OMSetRenderTargets(1, m_halfSizeRenderTarget.GetAddressOf(), nullptr);
}

void BloomPass::Setupsample()
{
    this->Unlink();
    m_info.samplingInfo = { 1.0f, 1.0f, 1.0f, 1.0f };
    m_samplingInfoBuffer.SetData(D3D11Core::Get().DeviceContext(), m_info.samplingInfo);
    D3D11Core::Get().DeviceContext()->PSSetShaderResources(1, 1, m_halfSizeView.GetAddressOf());
    D3D11Core::Get().DeviceContext()->OMSetRenderTargets(1, m_fullSizeTarget.GetAddressOf(), nullptr);
}

void BloomPass::Draw()
{
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
	m_blurPass.Create(BlurLevel::HIGH, BlurType::BOX);
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
    D3D11Core::Get().Device()->CreateTexture2D(&texDesc, nullptr, m_fullSize.GetAddressOf());
    D3D11Core::Get().Device()->CreateUnorderedAccessView(m_fullSize.Get(), NULL, m_fullSizeView.GetAddressOf());
    D3D11Core::Get().Device()->CreateRenderTargetView(m_fullSize.Get(), NULL, m_fullSizeTarget.GetAddressOf());

    D3D11Core::Get().Device()->CreateTexture2D(&texDesc, nullptr, m_blurredTexture.GetAddressOf());
    D3D11Core::Get().Device()->CreateShaderResourceView(m_blurredTexture.Get(), NULL, m_blurredView.GetAddressOf());
    D3D11Core::Get().Device()->CreateUnorderedAccessView(m_blurredTexture.Get(), NULL, m_blurredAccess.GetAddressOf());


    texDesc.Height /= 4;
    texDesc.Width /= 4;
    D3D11Core::Get().Device()->CreateTexture2D(&texDesc, nullptr, m_halfSize.GetAddressOf());
    D3D11Core::Get().Device()->CreateShaderResourceView(m_halfSize.Get(), NULL, m_halfSizeView.GetAddressOf());
    D3D11Core::Get().Device()->CreateRenderTargetView(m_halfSize.Get(), NULL, m_halfSizeRenderTarget.GetAddressOf());
    backBuff->Release();
}

void BloomPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    ID3D11Texture2D* backBuff = nullptr;
    HRESULT hr = D3D11Core::Get().SwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuff));
    if (FAILED(hr))
    {
        backBuff->Release();
        return;
    }
    DC->CopyResource(m_fullSize.Get(), backBuff);
    backBuff->Release();
   
    m_blurPass.BlurTexture(m_fullSizeView.GetAddressOf(), m_blurredAccess.GetAddressOf(), D3D11Core::Get().DeviceContext());
}

void BloomPass::Render(Scene* pScene)
{
    this->Setdownsample();
    this->Draw();
    this->Setupsample();
    this->Draw();
}

void BloomPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
}
