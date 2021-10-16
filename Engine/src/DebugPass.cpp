#include "EnginePCH.h"
#include "BasePass.h"

#include "PipelineManager.h"
#include "RModel.h"

void DebugPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DC->IASetInputLayout(PM->m_defaultInputLayout.Get());

    DC->VSSetShader(PM->m_defaultVertexShader.Get(), nullptr, 0);
    DC->PSSetShader(PM->m_debugPixelShader.Get(), nullptr, 0);

    DC->VSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());

    DC->RSSetViewports(1, &PM->m_viewport);
    DC->RSSetState(PM->m_rasterStateWireframe.Get());

    DC->OMSetRenderTargets(1, PM->m_backBuffer.GetAddressOf(), PM->m_debugDepthStencilView.Get());
    //DC->OMSetDepthStencilState(PM->m_depthStencilStateEqualAndDisableDepthWrite.Get(), 0);
}

void DebugPass::Render(Scene* pScene)
{
    pScene->RenderDebug();
}

void DebugPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    // Cleanup.
    //ID3D11ShaderResourceView* nullSRV[] = { nullptr };
    //DC->PSSetShaderResources(0, 1, nullSRV);
}

