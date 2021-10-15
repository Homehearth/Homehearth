#include "EnginePCH.h"
#include "Lights.h"

const bool Lights::SetupLightBuffer()
{
    HRESULT hr;

    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = sizeof(light_t) * (UINT)m_lights.size();
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.StructureByteStride = sizeof(light_t);
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem = &m_lights[0];
    hr = D3D11Core::Get().Device()->CreateBuffer(&desc, &data, &m_lightBuffer);
    
    if (!(FAILED(hr)))
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.BufferEx.FirstElement = 0;
        srvDesc.BufferEx.Flags = 0;
        srvDesc.BufferEx.NumElements = (UINT)m_lights.size();

        hr = D3D11Core::Get().Device()->CreateShaderResourceView(m_lightBuffer.Get(), &srvDesc, &m_lightShaderView);

        return !FAILED(hr);
    }

    return false;
}

const bool Lights::SetupInfoBuffer()
{
    HRESULT hr;

    D3D11_BUFFER_DESC infoDesc = {};
    infoDesc.Usage = D3D11_USAGE_DYNAMIC;
    infoDesc.ByteWidth = sizeof(light_info_t);
    infoDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    infoDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    infoDesc.MiscFlags = 0;

    hr = D3D11Core::Get().Device()->CreateBuffer(&infoDesc, NULL, &m_lightInfoBuffer);
    return !FAILED(hr);
}

const bool Lights::UpdateInfoBuffer()
{
    light_info_t newInfo = {};
    newInfo.nrOfLights = dx::XMFLOAT4(m_lights.size(), 0.f, 0.f, 0.f);

    D3D11Core::Get().DeviceContext()->UpdateSubresource(m_lightInfoBuffer.Get(), 3, NULL, &newInfo, 0, 0);
    return true;
}

Lights::Lights()
{
    m_lights = {};
    m_lightBuffer = nullptr;
    m_lightInfoBuffer = nullptr;
    m_lightShaderView = nullptr;
}

Lights::~Lights()
{

}

bool Lights::Initialize()
{
    light_t L;
    L.position = sm::Vector4(0.f, 8.f, -10.f, 1.f);
    L.direction = sm::Vector4(0.f, -1.f, 1.f, 0.f);
    L.color = sm::Vector4(300.f, 300.f, 300.f, 300.f);
    L.range = 75.f;
    L.enabled = 1;
    L.type = 0;

    m_lights.push_back(L);

    if (!SetupLightBuffer())
        return false;
    if (!SetupInfoBuffer())
        return false;

    return true;
}

void Lights::Render()
{
    UpdateInfoBuffer();
    D3D11Core::Get().DeviceContext()->PSSetConstantBuffers(3, 1, &m_lightInfoBuffer);
    D3D11Core::Get().DeviceContext()->PSSetShaderResources(7, 1, &m_lightShaderView);
}
