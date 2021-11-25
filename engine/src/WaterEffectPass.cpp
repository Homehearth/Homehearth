#include "EnginePCH.h"
#include "WaterEffectPass.h"
#include "PipelineManager.h"



void TextureEffectPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    // INPUT ASSEMBLY.(Dummy)
    {
        DC->IASetInputLayout(PM->m_defaultInputLayout.Get());
    }

    // SHADER STAGES.
    {
        DC->VSSetShader(PM->m_WaterEffectVertexShader.Get(), nullptr, 0);
        DC->CSSetShader(PM->m_WaterEffectComputeShader.Get(), nullptr, 0);
        DC->PSSetShader(PM->m_WaterEffectPixelShader.Get(), nullptr, 0);
        DC->GSSetShader(nullptr, nullptr, 0);
        DC->HSSetShader(nullptr, nullptr, 0);
        DC->DSSetShader(nullptr, nullptr, 0);

    }

    // CONSTANT BUFFERS.
    {
        DC->PSSetConstantBuffers(0, 0, nullptr);
        DC->VSSetConstantBuffers(0, 0, nullptr);
        DC->CSSetConstantBuffers(7, 1, PM->m_textureEffectConstantBuffer.GetAddressOf());
    }


    // SHADER RESOURCES.
    {
        //DC->CSSetShaderResources(17, 1, PM->m_SRV_TextureEffectBlendMap.GetAddressOf());
        //DC->CSSetShaderResources(18, 1, PM->m_SRV_TextureEffectWaterEdgeMap.GetAddressOf());
        //DC->CSSetShaderResources(19, 1, PM->m_SRV_TextureEffectWaterFloorMap.GetAddressOf());
        DC->CSSetShaderResources(20, 1, PM->m_SRV_TextureEffectWaterMap.GetAddressOf());
        DC->CSSetShaderResources(21, 1, PM->m_SRV_TextureEffectWaterNormalMap.GetAddressOf());
        DC->CSSetSamplers(0, 1, PM->m_pointSamplerState.GetAddressOf());
    }

    // BIND URV:s (Water and water normals)
    {
        DC->CSSetUnorderedAccessViews(5, 1, PM->m_UAV_TextureEffectWaterMap.GetAddressOf(), nullptr);
        DC->CSSetUnorderedAccessViews(5, 1, PM->m_UAV_TextureEffectWaterNormalMap.GetAddressOf(), nullptr);
    }

    // DISPATCH
    {
        DC->Dispatch(8, 8, 1);
    }

    // UNBIND SRV:S
    {
        ID3D11ShaderResourceView* const kill = { nullptr };
        DC->CSSetShaderResources(20, 1, &kill);
        DC->CSSetShaderResources(21, 1, &kill);
    }
}

void TextureEffectPass::Render(Scene* pScene)
{
    // Update constantbuffer here!
    m_CBuffer.amplitude = 0.05f;
    m_CBuffer.frequency = 25.f;
    m_CBuffer.counter += Stats::Get().GetFrameTime();
    D3D11Core::Get().DeviceContext()->UpdateSubresource(PM->m_textureEffectConstantBuffer.Get(), 0, nullptr, &m_CBuffer, 0, 0);
}

void TextureEffectPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    //Unbind SRV again.
    ID3D11ShaderResourceView* const kill = { nullptr };
    DC->CSSetShaderResources(20, 1, &kill);
    DC->CSSetShaderResources(21, 1, &kill);
}




