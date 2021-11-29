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
        DC->CSSetConstantBuffers(7, 1, PM->m_textureEffectConstantBuffer.GetAddressOf());
    }

    
    // SHADER RESOURCES.
    {
        //DC->CSSetShaderResources(18, 1, PM->m_SRV_TextureEffectWaterEdgeMap.GetAddressOf());
        DC->CSSetShaderResources(19, 1, PM->m_SRV_TextureEffectWaterFloorMap.GetAddressOf());
        DC->CSSetSamplers(0, 1, PM->m_pointSamplerState.GetAddressOf());
    }

    // BIND URV:s
    {
        DC->CSSetUnorderedAccessViews(5, 1, PM->m_UAV_TextureEffectWaterFloorMap.GetAddressOf(), nullptr);
    }

    // DISPATCH
    {
        //const int groupCount = static_cast<int>(ceil(m_MAX_PIXELS));
        DC->Dispatch(8, 8, 1);
    }
}

void TextureEffectPass::Render(Scene* pScene)
{
    //Reset timer at some point
    if (m_CBuffer.counter > 1000)
    {
        m_CBuffer.counter = 0;
    }

    // Update constantbuffer here!
    m_CBuffer.amplitude = 10.f;
    m_CBuffer.frequency = 25.f;
    m_CBuffer.counter += Stats::Get().GetFrameTime();
    D3D11Core::Get().DeviceContext()->UpdateSubresource(PM->m_textureEffectConstantBuffer.Get(), 0, nullptr, &m_CBuffer, 0, 0);
}

void TextureEffectPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    // UNBIND SRV:S
    {
        ID3D11ShaderResourceView* const kill = { nullptr };
        DC->CSSetShaderResources(19, 1, &kill);
    }

    //UNBIND URV:S
    {
        ID3D11UnorderedAccessView* const kill = { nullptr };
        DC->CSSetUnorderedAccessViews(5, 1, &kill, nullptr);
    }

    //UNBIUND SHADER:S
    {
        ID3D11VertexShader* kill = nullptr;
        ID3D11PixelShader* kill2 = nullptr;
        ID3D11ComputeShader* kill3 = nullptr;

        DC->VSSetShader(kill, nullptr, 0);
        DC->PSSetShader(kill2, nullptr, 0);
        DC->CSSetShader(kill3, nullptr, 0);
    }
}




