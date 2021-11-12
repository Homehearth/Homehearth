#include "EnginePCH.h"
#include "FrustumPass.h"

void FrustumPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    ID3D11RenderTargetView* nullRTV[] = { nullptr };
    ID3D11DepthStencilView* nullDSV = { nullptr };

    DC->OMSetRenderTargets(1, nullRTV, nullDSV);
    
    DC->VSSetShader(nullptr, nullptr, 0);
    DC->PSSetShader(nullptr, nullptr, 0);
    DC->CSSetShader(PM->m_computeFrustums.Get(), nullptr, 0);

    ID3D11Buffer* const buffers[] = { PM->m_dispatchParamsCB.GetBuffer() };
    DC->CSSetConstantBuffers(7, 1, buffers);
    DC->CSSetUnorderedAccessViews(0, 1, PM->m_outFrustumsUAV.GetAddressOf(), nullptr);
}

void FrustumPass::Render(Scene* pScene)
{
    GetDeviceContext()->Dispatch(m_dispatchParams.numThreads.x, m_dispatchParams.numThreads.y, 1);
}

void FrustumPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    // Cleanup.
    ID3D11UnorderedAccessView* nullUAV[] = { nullptr };
    DC->CSSetShader(nullptr, nullptr, 0);
    DC->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);

    // Only run this pass once.
    SetEnable(false);
}

bool FrustumPass::ComputeGridFrustums()
{
    // Safety check: screen resolution must be > 0 so at least 1 thread is created.
    const auto screenWidth = static_cast<uint32_t>(max(PM->m_viewport.Width, 1u));
    const auto screenHeight = static_cast<uint32_t>(max(PM->m_viewport.Height, 1u));

    const dx::XMUINT3 numThreads = { screenWidth / TILE_SIZE, screenHeight / TILE_SIZE, 1, };
    const dx::XMUINT3 numThreadGroups = { numThreads.x / TILE_SIZE, numThreads.y / TILE_SIZE, 1 };

    // Update DispatchParams
    m_dispatchParams.numThreadGroups = numThreadGroups;
    m_dispatchParams.numThreads = numThreads;
    PM->m_dispatchParamsCB.SetData(GetDeviceContext(), m_dispatchParams);

    // We will need 1 frustum for each grid cell.
    // [x, y] screen resolution and [z, z] tile size yield [x/z, y/z] grid size.
    // Resulting in a total of x/z * y/z frustums.
    m_numFrustums = { screenWidth / TILE_SIZE * screenHeight / TILE_SIZE };

    // Create a RWStructuredBuffer for storing the grid frustums.
    // m_inFrustumsSRV and m_outFrustumsUAV.
    if (!CreateGridFrustumSWB(m_numFrustums, sizeof(frustum_t)))
    {
        LOG_ERROR("CreateGridFrustumSWB() failed.");
        return false;
    }

    return true;
}

bool FrustumPass::CreateGridFrustumSWB(size_t numElements, size_t elementSize, const void* bufferData) const
{
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));

    bufferDesc.ByteWidth = static_cast<UINT>(numElements * elementSize);
    bufferDesc.StructureByteStride = static_cast<UINT>(elementSize);
    bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_FLAG::D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    D3D11_SUBRESOURCE_DATA subData;
    ZeroMemory(&subData, sizeof(D3D11_SUBRESOURCE_DATA));

    subData.pSysMem = bufferData;
    subData.SysMemPitch = 0;
    subData.SysMemSlicePitch = 0;

    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&bufferDesc, nullptr, PM->m_frustums_buffer.GetAddressOf());
    if (FAILED(hr))
        return false;

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.Flags = 0;
    uavDesc.Buffer.NumElements = static_cast<UINT>(numElements);
    hr = D3D11Core::Get().Device()->CreateUnorderedAccessView(PM->m_frustums_buffer.Get(), &uavDesc, PM->m_outFrustumsUAV.GetAddressOf());
    if (FAILED(hr))
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    srvDesc.BufferEx.NumElements = static_cast<UINT>(numElements);
    hr = D3D11Core::Get().Device()->CreateShaderResourceView(PM->m_frustums_buffer.Get(), &srvDesc, PM->m_inFrustumsSRV.GetAddressOf());

    return !FAILED(hr);
}

bool FrustumPass::UpdateFrustums()
{
    return ComputeGridFrustums();
}
