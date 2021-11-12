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
        
        DC->CSSetShaderResources(1, 17, PM->m_SRV_TextureEffectWaterFloorMap.GetAddressOf());
        DC->CSSetShaderResources(1, 18, PM->m_SRV_TextureEffectWaterEdgeMap.GetAddressOf());
        DC->CSSetShaderResources(1, 19, PM->m_SRV_TextureEffectWaterMap.GetAddressOf());
        DC->CSSetShaderResources(1, 20, PM->m_SRV_TextureEffectBlendMap.GetAddressOf());
        DC->CSSetShaderResources(1, 21, PM->m_SRV_TextureEffectWaterNormalMap.GetAddressOf());
        DC->CSSetSamplers(0, 1, PM->m_pointSamplerState.GetAddressOf());
    }

    // OUTPUT MERGER.
    {
        DC->OMSetRenderTargets(1, PM->m_renderTargetView.GetAddressOf(), PM->m_depthStencilView.Get());
        DC->OMSetBlendState(PM->m_blendStatepOpaque.Get(), nullptr, 0xFFFFFFFF);
        DC->OMSetDepthStencilState(PM->m_depthStencilStateLess.Get(), 1);
    }

    //dispatch here

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
}


