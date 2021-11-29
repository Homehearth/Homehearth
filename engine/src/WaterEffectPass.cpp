#include "EnginePCH.h"
#include "WaterEffectPass.h"
#include "PipelineManager.h"



void WaterEffectPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    //UNBIND UAV:S
    {
        ID3D11UnorderedAccessView* const kill = { nullptr };
        DC->CSSetUnorderedAccessViews(5, 1, &kill, nullptr);
        DC->CSSetUnorderedAccessViews(6, 1, &kill, nullptr);
        DC->CSSetUnorderedAccessViews(7, 1, &kill, nullptr);
    }

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
        //DC->CSSetUnorderedAccessViews(5, 1, PM->m_UAV_TextureEffectBlendMap.GetAddressOf(), nullptr);
        DC->CSSetUnorderedAccessViews(6, 1, PM->m_UAV_TextureEffectWaterMap.GetAddressOf(), nullptr);
        DC->CSSetUnorderedAccessViews(7, 1, PM->m_UAV_TextureEffectWaterNormalMap.GetAddressOf(), nullptr);
    }

    // DISPATCH
    {
        DC->Dispatch(8, 8, 1);
    }
}

void WaterEffectPass::Render(Scene* pScene)
{
    //Reset timer at some point
    if (m_CBuffer.counter > 1000)
    {
        m_CBuffer.counter = 0;
    }

    // Update constantbuffer here!
    m_CBuffer.amplitude = 10.f;
    m_CBuffer.frequency = 250.f;
    m_CBuffer.counter += Stats::Get().GetFrameTime();

    D3D11Core::Get().DeviceContext()->UpdateSubresource(PM->m_textureEffectConstantBuffer.Get(), 0, nullptr, &m_CBuffer, 0, 0);
}

void WaterEffectPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    // UNBIND SRV:S
    {
        ID3D11ShaderResourceView* const kill = { nullptr };
        //DC->CSSetShaderResources(17, 1, &kill);
        DC->CSSetShaderResources(20, 1, &kill);
        DC->CSSetShaderResources(21, 1, &kill);
    }

    //UNBIND UAV:S
    {
        ID3D11UnorderedAccessView* const kill = { nullptr };
        //DC->CSSetUnorderedAccessViews(5, 1, &kill, nullptr);
        DC->CSSetUnorderedAccessViews(6, 1, &kill, nullptr);
        DC->CSSetUnorderedAccessViews(7, 1, &kill, nullptr);
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




