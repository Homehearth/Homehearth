#include "EnginePCH.h"
#include "TextureEffectPass.h"

#include "PipelineManager.h"
//#include "RMesh.h"



void TextureEffectPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{ 
    // INPUT ASSEMBLY.
    {
        DC->IASetInputLayout(PM->m_defaultInputLayout.Get());
    }

    // SHADER STAGES.
    {
        DC->VSSetShader(PM->m_textureEffectVertexShader.Get(),  nullptr, 0);
        DC->CSSetShader(PM->m_textureEffectComputeShader.Get(), nullptr, 0);
        DC->PSSetShader(PM->m_textureEffectPixelShader.Get(),   nullptr, 0);
        DC->GSSetShader(nullptr, nullptr, 0);
        DC->HSSetShader(nullptr, nullptr, 0);
        DC->DSSetShader(nullptr, nullptr, 0);

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
        DC->CSSetShaderResources(17, 1, PM->m_SRV_TextureEffectBlendMap.GetAddressOf());
        DC->CSSetShaderResources(18, 1, PM->m_SRV_TextureEffectWaterEdgeMap.GetAddressOf());
        DC->CSSetShaderResources(19, 1, PM->m_SRV_TextureEffectWaterFloorMap.GetAddressOf());
        DC->CSSetShaderResources(20, 1, PM->m_SRV_TextureEffectWaterMap.GetAddressOf());
        DC->CSSetShaderResources(21, 1, PM->m_SRV_TextureEffectWaterNormalMap.GetAddressOf());
        DC->CSSetSamplers(0, 1, PM->m_pointSamplerState.GetAddressOf());
    }

    // DISPATCH
    {
        const int groupCount = static_cast<int>(ceil(m_MAX_PIXELS / 1024));
        DC->Dispatch(groupCount, 1, 1);
    }

    // UNBIND SRV:S
    {
        ID3D11ShaderResourceView* const kill[5] = { nullptr };
        pDeviceContext->CSSetShaderResources(17, 5, kill);

        for (int i = 0; i < 5; i++)
        {
            delete kill[i];
        }
    }

    // OUTPUT MERGER. BIND RTV:S
    {
        ID3D11RenderTargetView* targets[5] =
        {
            PM->m_RTV_TextureEffectBlendMap.Get(),      //slot 0 - 17 Blendmap: 225 x 225
            PM->m_RTV_TextureEffectWaterEdgeMap.Get(),  //slot 1 - 18 WaterEdge: 1024 x 1024
            PM->m_RTV_TextureEffectWaterFloorMap.Get(), //slot 2 - 19 WaterFloor: 256 x 256
            PM->m_RTV_TextureEffectWaterMap.Get(),      //slot 3 - 20 Water: 256 x 256
            PM->m_RTV_TextureEffectWaterNormalMap.Get() //slot 4 - 21 WaterNormal: 600 x 600
        };

        DC->OMSetRenderTargets(5, targets, nullptr);
        //Might need to set view port
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
    
    DC->ClearRenderTargetView(PM->m_RTV_TextureEffectBlendMap.Get(),        m_clearColor);
    DC->ClearRenderTargetView(PM->m_RTV_TextureEffectWaterEdgeMap.Get(),    m_clearColor);
    DC->ClearRenderTargetView(PM->m_RTV_TextureEffectWaterFloorMap.Get(),   m_clearColor);
    DC->ClearRenderTargetView(PM->m_RTV_TextureEffectWaterMap.Get(),        m_clearColor);
    DC->ClearRenderTargetView(PM->m_RTV_TextureEffectWaterNormalMap.Get(),  m_clearColor);

    ID3D11RenderTargetView* const kill[5] = { nullptr };
    pDeviceContext->OMSetRenderTargets(5, kill, nullptr);

    for (int i = 0; i < 5; i++)
    {
        delete kill[i];
    }

    ID3D11ShaderResourceView* const kill2[5] = { nullptr };
    pDeviceContext->CSSetShaderResources(17, 5, kill2);

    for (int i = 0; i < 5; i++)
    {
        delete kill2[i];
    }
}




