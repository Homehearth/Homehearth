#include "EnginePCH.h"
#include "CullingPass.h"

void CullingPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    DC->VSSetShader(nullptr, nullptr, 0);      
    DC->PSSetShader(nullptr, nullptr, 0);     

    DC->CSSetSamplers(4, 1, PM->m_linearClampSamplerState.GetAddressOf());
	DC->CSSetShader(PM->m_lightCullingShader.Get(), nullptr, 0);

    DC->CSSetShaderResources(0, 1, PM->m_depth.srv.GetAddressOf());
    DC->CSSetShaderResources(24, 1, PM->m_frustums.srv.GetAddressOf());
    DC->CSSetShaderResources(26, 1, &PM->m_lightCountHeatMap.get()->GetShaderView());

    ID3D11Buffer* const buffers[] = {
		PM->m_screenToViewParamsCB.GetBuffer(),
		PM->m_dispatchParamsCB.GetBuffer()
    };
    DC->CSSetConstantBuffers(11, ARRAYSIZE(buffers), buffers); // ScreenToViewParamsCB + DispatchParamsCB
    DC->CSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());  // Camera.

	m_lights->Render(DC, true);

    PM->trans_LightIndexCounter_data[0] = 0;
    PM->opaq_LightIndexCounter_data[0] = 0;

    PM->BindStructuredBuffer(1, 1, PM->opaq_LightIndexCounter.buffer.Get(), 
        PM->opaq_LightIndexCounter_data.data(),PM->opaq_LightIndexCounter.uav.GetAddressOf());

    PM->BindStructuredBuffer(2, 1, PM->trans_LightIndexCounter.buffer.Get(),
        PM->trans_LightIndexCounter_data.data(), PM->trans_LightIndexCounter.uav.GetAddressOf());

    PM->BindStructuredBuffer(3, 1, PM->opaq_LightIndexList.buffer.Get(),
        PM->opaq_LightIndexList_data.data(), PM->opaq_LightIndexList.uav.GetAddressOf());

    PM->BindStructuredBuffer(4, 1, PM->trans_LightIndexList.buffer.Get(),
        PM->trans_LightIndexList_data.data(), PM->trans_LightIndexList.uav.GetAddressOf());

    DC->CSSetUnorderedAccessViews(5, 1, PM->opaq_LightGrid.uav.GetAddressOf(), nullptr);
    DC->CSSetUnorderedAccessViews(6, 1, PM->trans_LightGrid.uav.GetAddressOf(), nullptr);
    DC->CSSetUnorderedAccessViews(7, 1, PM->m_heatMap.uav.GetAddressOf(), nullptr);
}   

void CullingPass::Render(Scene* pScene)
{
    PROFILE_FUNCTION();

    GetDeviceContext()->Dispatch(PM->m_dispatchParams.numThreads.x, PM->m_dispatchParams.numThreads.y, 1);
}

void CullingPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    ID3D11ShaderResourceView* nullSRV[] = { nullptr };
    ID3D11UnorderedAccessView* nullUAV[] = { nullptr };

    DC->CSSetShader(nullptr, nullptr, 0);
    DC->CSSetShaderResources(0, 1, nullSRV);
    DC->CSSetShaderResources(24, 1, nullSRV);
    DC->CSSetShaderResources(26, 1, nullSRV);

    DC->CSSetUnorderedAccessViews(1, 1, nullUAV, nullptr);
    DC->CSSetUnorderedAccessViews(2, 1, nullUAV, nullptr);
    DC->CSSetUnorderedAccessViews(3, 1, nullUAV, nullptr);
    DC->CSSetUnorderedAccessViews(4, 1, nullUAV, nullptr);
    DC->CSSetUnorderedAccessViews(5, 1, nullUAV, nullptr);
    DC->CSSetUnorderedAccessViews(6, 1, nullUAV, nullptr);
    DC->CSSetUnorderedAccessViews(7, 1, nullUAV, nullptr);
}
