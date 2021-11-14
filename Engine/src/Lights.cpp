#include "EnginePCH.h"
#include "Lights.h"

const bool Lights::SetupLightBuffer()
{
    int size = (int)m_lights.size() > 0 ? (int)m_lights.size():1;
    HRESULT hr;

    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = sizeof(light_t) * size;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.StructureByteStride = sizeof(light_t);
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    if (!m_lights.empty())
    {
        D3D11_SUBRESOURCE_DATA data = {};
        data.pSysMem = &m_lights[0];
        hr = D3D11Core::Get().Device()->CreateBuffer(&desc, &data, m_lightBuffer.GetAddressOf());
    }
    else
        return false;
    
    if (!(FAILED(hr)))
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.BufferEx.FirstElement = 0;
        srvDesc.BufferEx.Flags = 0;
        srvDesc.BufferEx.NumElements = (UINT)m_lights.size();

        hr = D3D11Core::Get().Device()->CreateShaderResourceView(m_lightBuffer.Get(), &srvDesc, m_lightShaderView.GetAddressOf());

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

    hr = D3D11Core::Get().Device()->CreateBuffer(&infoDesc, NULL, m_lightInfoBuffer.GetAddressOf());
    return !FAILED(hr);
}

const bool Lights::UpdateLightBuffer()
{
    HRESULT hr;
    D3D11_MAPPED_SUBRESOURCE submap;
    hr = D3D11Core::Get().DeviceContext()->Map(m_lightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &submap);
    memcpy(submap.pData, &(m_lights[0]), sizeof(light_t) * m_lights.size());
    D3D11Core::Get().DeviceContext()->Unmap(m_lightBuffer.Get(), 0);
    return !FAILED(hr);
}

const bool Lights::UpdateInfoBuffer()
{
    light_info_t newInfo = {};
    newInfo.nrOfLights = dx::XMFLOAT4((float)m_lights.size(), 0.f, 0.f, 0.f);

    HRESULT hr;
    D3D11_MAPPED_SUBRESOURCE submap;
    hr = D3D11Core::Get().DeviceContext()->Map(m_lightInfoBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &submap);
    memcpy(submap.pData, &newInfo, sizeof(light_info_t));
    D3D11Core::Get().DeviceContext()->Unmap(m_lightInfoBuffer.Get(), 0);
    return !FAILED(hr);
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
    if (!SetupLightBuffer())
    {
        return false;
    }

    if (!SetupInfoBuffer())
    {
        return false;
    }

    m_isInit = true;

    return m_isInit;
}

const bool Lights::IsInitialize() const
{
    return m_isInit;
}

void Lights::Render(ID3D11DeviceContext* dc)
{
    if (dc == D3D11Core::Get().DeviceContext() && m_isInit)
    {
        UpdateInfoBuffer();
        UpdateLightBuffer();
    }

    dc->PSSetConstantBuffers(4, 1, m_lightInfoBuffer.GetAddressOf());
    dc->PSSetShaderResources(T2D_SLOT, 1, m_lightShaderView.GetAddressOf());
}

void Lights::EditLight(light_t L, const int& index)
{
    if (index < (int)m_lights.size() && index >= 0)
    {
        m_lights[index] = L;
    }
}

void Lights::Add(entt::registry& reg, entt::entity ent)
{
    reg.get<comp::Light>(ent).index = (int)m_lights.size();
    m_lights.push_back(reg.get<comp::Light>(ent).lightData);
}
