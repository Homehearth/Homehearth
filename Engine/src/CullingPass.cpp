#include "EnginePCH.h"
#include "CullingPass.h"

void CullingPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    ID3D11RenderTargetView* nullRTV[] = { nullptr };
    ID3D11DepthStencilView* nullDSV = { nullptr };
   
    DC->OMSetRenderTargets(1, nullRTV, nullDSV);
    DC->OMSetDepthStencilState(PM->m_depthStencilStateLessOrEqual.Get(), 0);

    DC->VSSetShader(nullptr, nullptr, 0);      
    DC->PSSetShader(nullptr, nullptr, 0);     

    DC->CSSetSamplers(4, 1, PM->m_linearClampSamplerState.GetAddressOf());
	DC->CSSetShader(PM->m_lightCullingShader.Get(), nullptr, 0);
    DC->CSSetShaderResources(24, 1, PM->m_frustums.srv.GetAddressOf());
    DC->CSSetShaderResources(0, 1, PM->m_depth.srv.GetAddressOf());
    DC->CSSetUnorderedAccessViews(3, 1, PM->opaq_LightIndexList.uav.GetAddressOf(), nullptr);
    DC->CSSetUnorderedAccessViews(4, 1, PM->trans_LightIndexList.uav.GetAddressOf(), nullptr);
    DC->CSSetUnorderedAccessViews(5, 1, PM->opaq_LightGrid.uav.GetAddressOf(), nullptr);
    DC->CSSetUnorderedAccessViews(6, 1, PM->trans_LightGrid.uav.GetAddressOf(), nullptr);
    DC->CSSetUnorderedAccessViews(7, 1, PM->m_heatMap.uav.GetAddressOf(), nullptr);

    PM->trans_LightIndexCounter_data[0] = 0;
    PM->opaq_LightIndexCounter_data[0] = 0;
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
    DC->CSSetShaderResources(24, 1, nullSRV);
    DC->CSSetShaderResources(0, 1, nullSRV);
    DC->CSSetUnorderedAccessViews(3, 1, nullUAV, nullptr);
    DC->CSSetUnorderedAccessViews(4, 1, nullUAV, nullptr);
    DC->CSSetUnorderedAccessViews(5, 1, nullUAV, nullptr);
    DC->CSSetUnorderedAccessViews(6, 1, nullUAV, nullptr);
    DC->CSSetUnorderedAccessViews(7, 1, nullUAV, nullptr);
}
