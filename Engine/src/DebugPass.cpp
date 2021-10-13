#include "EnginePCH.h"
#include "BasePass.h"

#include "PipelineManager.h"
#include "RModel.h"

void DebugPass::PreRender(ID3D11DeviceContext* pDeviceContext)
{
    DC->IASetInputLayout(PM->m_defaultInputLayout.Get());
    DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	
    DC->VSSetShader(PM->m_defaultVertexShader.Get(), nullptr, 0);
    DC->PSSetShader(PM->m_defaultPixelShader.Get(), nullptr, 0);

    DC->VSSetConstantBuffers(1, 1, CAMERA->m_viewConstantBuffer.GetAddressOf());

    // DC->PSSetShaderResources(0, 1, PM->m_depthBufferSRV.GetAddressOf());   // DepthBuffer.

    DC->PSSetSamplers(0, 1, PM->m_linearSamplerState.GetAddressOf());
    DC->PSSetSamplers(1, 1, PM->m_pointSamplerState.GetAddressOf());

    DC->RSSetState(PM->m_rasterState.Get());

    DC->OMSetRenderTargets(1, PM->m_backBuffer.GetAddressOf(), PM->m_depthStencilView.Get());
    DC->OMSetDepthStencilState(PM->m_depthStencilStateEqualAndDisableDepthWrite.Get(), 0);
}

void DebugPass::Render(Scene* pScene)
{
    pScene->Render();
}

void DebugPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    // Cleanup.
    //ID3D11ShaderResourceView* nullSRV[] = { nullptr };
    //DC->PSSetShaderResources(0, 1, nullSRV);
}

