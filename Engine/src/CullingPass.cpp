#include "EnginePCH.h"
#include "CullingPass.h"

void CullingPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    ID3D11RenderTargetView* nullRTV[] = { nullptr };
    ID3D11DepthStencilView* nullDSV = { nullptr };
   
    DC->OMSetRenderTargets(1, nullRTV, nullDSV);                
    DC->VSSetShader(nullptr, nullptr, 0);      
    DC->PSSetShader(nullptr, nullptr, 0);     
    
	DC->CSSetShader(PM->m_lightCullingShader.Get(), nullptr, 0);
    DC->CSSetShaderResources(0, 1, );
    DC->CSSetShaderResources(1, 1, );
    DC->CSSetShaderResources(2, 1, PM->m_depthBufferSRV.GetAddressOf());
    DC->CSSetUnorderedAccessViews(0, 1, , nullptr);

}   

void CullingPass::Render(Scene* pScene)
{
    GetDeviceContext()->Dispatch(PM->m_dispatchParams.numThreads.x, PM->m_dispatchParams.numThreads.y, 1);
    LOG_WARNING("CullingPass executed.")
}

void CullingPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    ID3D11ShaderResourceView* nullSRV[] = { nullptr };
    ID3D11UnorderedAccessView* nullUAV[] = { nullptr };

    DC->CSSetShader(nullptr, nullptr, 0);
    DC->CSSetShaderResources(0, 1, nullSRV);
    DC->CSSetShaderResources(1, 1, nullSRV);
    DC->CSSetShaderResources(2, 1, nullSRV);
    DC->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
}
