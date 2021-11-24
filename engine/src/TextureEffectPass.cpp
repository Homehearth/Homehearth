#include "EnginePCH.h"
#include "TextureEffectPass.h"

#include "PipelineManager.h"



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
        //DC->CSSetShaderResources(17, 1, PM->m_SRV_TextureEffectBlendMap.GetAddressOf());
        //DC->CSSetShaderResources(18, 1, PM->m_SRV_TextureEffectWaterEdgeMap.GetAddressOf());
        DC->CSSetShaderResources(19, 1, PM->m_SRV_TextureEffectWaterFloorMap.GetAddressOf());
       /* DC->CSSetShaderResources(20, 1, PM->m_SRV_TextureEffectWaterMap.GetAddressOf());
        DC->CSSetShaderResources(21, 1, PM->m_SRV_TextureEffectWaterNormalMap.GetAddressOf());*/
        DC->CSSetSamplers(0, 1, PM->m_pointSamplerState.GetAddressOf());
    }

    // BIND URV:s
    {
        DC->CSSetUnorderedAccessViews(5, 1, PM->m_UAV_TextureEffectWaterFloorMap.GetAddressOf(), nullptr);
    }

    // DISPATCH
    {
        const int groupCount = static_cast<int>(ceil(m_MAX_PIXELS));
        DC->Dispatch(8, 8, 1);
    }

    // UNBIND SRV:S
    {
        ID3D11ShaderResourceView* const kill = { nullptr };
        DC->CSSetShaderResources(19, 1, &kill);
    }

    // RESET TEXTURES
    {

    }
}

void TextureEffectPass::Render(Scene* pScene)
{
    // Render objects.
    // pScene->Render();
}

void TextureEffectPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    // Update constantbuffer here!
    m_CBuffer.amplitude = 0.05f;
    m_CBuffer.frequency = 25.f;
    D3D11Core::Get().DeviceContext()->UpdateSubresource(PM->m_textureEffectConstantBuffer.Get(), 0, nullptr, &m_CBuffer, 0, 0);
    
    //Unbind SRV again.
    ID3D11ShaderResourceView* const kill = { nullptr };
    pDeviceContext->CSSetShaderResources(19, 1, &kill);

}




