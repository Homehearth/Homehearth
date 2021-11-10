#include "EnginePCH.h"
#include "LightCullingPass.h"

void LightCullingPass::ComputeGridFrustums()
{
    // Safety check: screen resolution must be > 0 so at least 1 thread is created.
    uint32_t screenWidth = static_cast<uint32_t>(max(PM->m_viewport.Width, 1u));
    uint32_t screenHeight = static_cast<uint32_t>(max(PM->m_viewport.Width, 1u));

    dx::XMUINT3 numThreads = { screenWidth / TILE_SIZE, screenHeight / TILE_SIZE, 1, };
    dx::XMUINT3 numThreadGroups = { numThreads.x / TILE_SIZE, numThreads.y / TILE_SIZE, 1 };

    m_dispatchParams.numThreadGroups = numThreadGroups;
    m_dispatchParams.numThreads = numThreads;

    // We will need 1 frustum for each grid cell.
    // [x, y] screen resolution and [z,z] tile size yield [x/z, y/z] grid size.
    // Resulting in a total of x/z * y/z frustums.
    m_numFrustums = { screenWidth / TILE_SIZE * screenHeight / TILE_SIZE };

    // Create a RWStructuredBuffer for storing the grid frustums.
    if (!CreateGridFrustumSWB(m_numFrustums, sizeof(frustum_t)))
    {
        LOG_ERROR("CreateGridFrustumSWB() failed.");
        m_hasChanged = true;
    }
    else
        m_hasChanged = false;
}

bool LightCullingPass::CreateGridFrustumSWB(size_t numElements, size_t elementSize, const void* bufferData)
{
    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

    desc.ByteWidth = static_cast<UINT>(numElements * elementSize);
    desc.StructureByteStride = static_cast<UINT>(elementSize);
    desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_FLAG::D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    D3D11_SUBRESOURCE_DATA data;
    ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));

    data.pSysMem = bufferData;
    data.SysMemPitch = 0;
    data.SysMemSlicePitch = 0;

    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&desc, nullptr, PM->m_gridFrustum.GetAddressOf());
    if (FAILED(hr))
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
    ZeroMemory(&resourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

    resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    resourceViewDesc.BufferEx.NumElements = static_cast<UINT>(numElements);
    hr = D3D11Core::Get().Device()->CreateShaderResourceView(PM->m_gridFrustum.Get(), &resourceViewDesc, PM->m_gridFrustumSRV.GetAddressOf());

    return !FAILED(hr);
}

void LightCullingPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    // if the dimension of the grid changes (etc screen is resized),
    // the culling frustums has to be recomputed.
    if (m_hasChanged)
    {
        ComputeGridFrustums();
    }
    else
    {
        ID3D11RenderTargetView* nullRTV[] = { nullptr };
        ID3D11DepthStencilView* nullDSV = { nullptr };
        DC->OMSetRenderTargets(1, nullRTV, nullDSV);

        DC->VSSetShader(nullptr, nullptr, 0);
        DC->PSSetShader(nullptr, nullptr, 0);

        ID3D11ShaderResourceView* nullSRV[] = { nullptr };
        DC->PSSetShaderResources(0, 1, nullSRV);
        DC->PSSetShaderResources(1, 1, nullSRV);

        ID3D11SamplerState* nullSAMPLER[] = { nullptr };
        DC->PSSetSamplers(0, 1, nullSAMPLER);

        //DC->CSSetShader(, nullptr, 0);
        DC->CSSetShaderResources(0, 1, PM->m_gridFrustumSRV.GetAddressOf());
        DC->CSSetShaderResources(2, 1, PM->m_depthBufferSRV.GetAddressOf());

        DC->Dispatch(m_dispatchParams.numThreads.x, m_dispatchParams.numThreads.y, 1);
    }
}

void LightCullingPass::Render(Scene* pScene)
{
    //SetCullBack(true);
    // render opaq.
    //SetCullBack(false);
	//// render trans.
    //SetCullBack(true);
}

void LightCullingPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    // Cleanup.
    ID3D11ShaderResourceView* nullSRV[] = { nullptr };
    DC->CSSetShader(nullptr, nullptr, 0);
    DC->CSSetShaderResources(0, 1, nullSRV);
    DC->CSSetShaderResources(1, 1, nullSRV);
    DC->CSSetShaderResources(2, 1, nullSRV);
}
