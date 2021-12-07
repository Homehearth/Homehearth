#include "EnginePCH.h"
#include "WaterEffectPass.h"
#include "PipelineManager.h"



void WaterEffectPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
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
        ID3D11ShaderResourceView* temp[3]
        {
            PM->m_SRV_TextureEffectWaterMap.Get(),
            PM->m_SRV_TextureEffectWaterNormalMap.Get(),
            PM->m_SRV_TextureEffectBlendMap.Get()
        };

        DC->CSSetShaderResources(20, 3, temp);
    }

    // BIND URV:s (Water and water normals)
    {
        ID3D11UnorderedAccessView* temp[3]
        {
            PM->m_UAV_TextureEffectBlendMap.Get(),
            PM->m_UAV_TextureEffectWaterMap.Get(),
            PM->m_UAV_TextureEffectWaterNormalMap.Get()
        };

        DC->CSSetUnorderedAccessViews(5, 3, temp, nullptr);
    }

    // DISPATCH
    {
        DC->Dispatch(8, 8, 1);
    }
}

void WaterEffectPass::Render(Scene* pScene)
{
    PROFILE_FUNCTION();

    //Reset timer at some point
    if (m_CBuffer.counter > 100)
    {
        m_CBuffer.counter = 0;
    }

    // Update constantbuffer here!
    m_CBuffer.amplitude = 10;
    m_CBuffer.frequency = 250;
    m_CBuffer.counter += Stats::Get().GetFrameTime();

    D3D11Core::Get().DeviceContext()->UpdateSubresource(PM->m_textureEffectConstantBuffer.Get(), 0, nullptr, &m_CBuffer, 0, 0);
}

void WaterEffectPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    // UNBIND SRV:S
    {
        ID3D11ShaderResourceView* const kill[3] = { nullptr };
        DC->CSSetShaderResources(20, 3, kill);
    }

    //UNBIND UAV:S
    {
        ID3D11UnorderedAccessView* kill[3] = { nullptr };
        DC->CSSetUnorderedAccessViews(5, 3, kill, nullptr);
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




