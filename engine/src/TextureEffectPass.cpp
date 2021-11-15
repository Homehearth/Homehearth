#include "EnginePCH.h"
#include "TextureEffectPass.h"

#include "PipelineManager.h"
//#include "RMesh.h"


void TextureEffectPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{ 
    // INPUT ASSEMBLY.
    {
        DC->IASetInputLayout(PM->m_positionOnlyInputLayout.Get());
    }

    // SHADER STAGES.
    {
        DC->VSSetShader(PM->m_textureEffectVertexShader.Get(), nullptr, 0);
        DC->GSSetShader(nullptr, nullptr, 0);
        DC->HSSetShader(nullptr, nullptr, 0);
        DC->DSSetShader(nullptr, nullptr, 0);
        DC->CSSetShader(PM->m_textureEffectComputeShader.Get(), nullptr, 0);
        DC->PSSetShader(PM->m_textureEffectPixelShader.Get(), nullptr, 0);
    }

    // CONSTANT BUFFERS.
    {
        DC->PSSetConstantBuffers(0, 0, nullptr);
        DC->VSSetConstantBuffers(0, 0, nullptr);
        DC->CSSetConstantBuffers(6, 1, PM->m_deltaTimeBuffer.GetAddressOf());
        DC->CSSetConstantBuffers(3, 1, PM->m_textureEffectConstantBuffer.GetAddressOf());
    }

    // SHADER RESOURCES.
    {
        //Kolla i din deffered rendering hur man la sånnahär i en array. 
        DC->CSSetShaderResources(17, 5, resources); // 17 - 21 (order checked)
        DC->CSSetSamplers(0, 1, PM->m_pointSamplerState.GetAddressOf());
    }

    // OUTPUT MERGER.
    {
        DC->OMSetRenderTargets(1, PM->m_renderTargetView.GetAddressOf(), PM->m_depthStencilView.Get());
        DC->OMSetRenderTargets(5, targets, PM->m_debugDepthStencilView.Get()); //(order checked)
        DC->OMSetBlendState(PM->m_blendStatepOpaque.Get(), nullptr, 0xFFFFFFFF);
        DC->OMSetDepthStencilState(PM->m_depthStencilStateLess.Get(), 1);
        //Might need to set view port
    }

    // DISPATCH

    {
        const int groupCount = static_cast<int>(ceil(MAX_PIXELS / 1024));
        DC->Dispatch(groupCount, 1, 1);
    }

}

void TextureEffectPass::Render(Scene* pScene)
{
    // Render objects.
    pScene->Render();
}

void TextureEffectPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    // Update constantbuffer here!
    m_CBuffer.amplitude = 0.05f;
    m_CBuffer.frequency = 25.f;
    D3D11Core::Get().DeviceContext()->UpdateSubresource(PM->m_textureEffectConstantBuffer.Get(), 0, nullptr, &m_CBuffer, 0, 0);
    
    pDeviceContext->ClearRenderTargetView(PM->m_RTV_TextureEffectBlendMap.Get(), clearColor);
    pDeviceContext->ClearRenderTargetView(PM->m_RTV_TextureEffectWaterEdgeMap.Get(), clearColor);
    pDeviceContext->ClearRenderTargetView(PM->m_RTV_TextureEffectWaterFloorMap.Get(), clearColor);
    pDeviceContext->ClearRenderTargetView(PM->m_RTV_TextureEffectWaterMap.Get(), clearColor);
    pDeviceContext->ClearRenderTargetView(PM->m_RTV_TextureEffectWaterNormalMap.Get(), clearColor);
}



