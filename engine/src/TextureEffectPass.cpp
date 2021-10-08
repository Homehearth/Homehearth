#include "EnginePCH.h"
#include "TextureEffectPass.h"

#include "PipelineManager.h"
#include "RMesh.h"

void TextureEffectPass::Initialize()
{
    // Initialize Pass.
    // accept scene.
    // accept materials.
    // accept ...
}

void TextureEffectPass::PreRender(ID3D11DeviceContext* dc, PipelineManager* pm)
{
    m_pm = pm;

    // INPUT ASSEMBLY.
    {
        dc->IASetInputLayout(pm->m_positionOnlyInputLayout.Get());
    }

    // SHADER STAGES.
    {
        dc->VSSetShader(pm->m_textureEffectVertexShader.Get(),  nullptr, 0); 
        dc->GSSetShader(nullptr, nullptr, 0);
        dc->HSSetShader(nullptr, nullptr, 0);
        dc->DSSetShader(nullptr, nullptr, 0);
        dc->CSSetShader(pm->m_textureEffectComputeShader.Get(), nullptr, 0);
        dc->PSSetShader(pm->m_textureEffectPixelShader.Get(),   nullptr, 0);
    }

    // CONSTANT BUFFERS.
    {
        dc->PSSetConstantBuffers(0, 0, nullptr);
        dc->VSSetConstantBuffers(0, 0, nullptr);
        dc->VSSetConstantBuffers(0, 0, nullptr);
        dc->CSSetConstantBuffers(0, 1, pm->m_textureEffectConstantBuffer.GetAddressOf());
    }

    // SHADER RESOURCES.
    {
        dc->PSSetShaderResources(0, 0, nullptr);
        dc->VSSetShaderResources(0, 0, nullptr);
        dc->PSSetSamplers(0, 1, pm->m_linearSamplerState.GetAddressOf());
    }

    // OUTPUT MERGER.
    {
        dc->OMSetRenderTargets(1, pm->m_renderTargetView.GetAddressOf(), pm->m_depthStencilView.Get());
        dc->OMSetBlendState(pm->m_blendStatepOpaque.Get(), nullptr, 0xFFFFFFFF);
        dc->OMSetDepthStencilState(pm->m_depthStencilStateLess.Get(), 1);
    }
}

void TextureEffectPass::Render(Scene* pScene)
{
    // Render objects.
    pScene->Render();
}

void TextureEffectPass::PostRender()
{
    // Update constantbuffer here!
    //D3D11Core::Get().DeviceContext()->UpdateSubresource(m_pm->m_textureEffectConstantBuffer.Get(), 0, nullptr, &m_CBuffer, 0, 0);
}

