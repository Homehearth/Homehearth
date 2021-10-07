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
    // INPUT ASSEMBLY.
    {
        dc->IASetInputLayout(nullptr);
    }

    // SHADER STAGES.
    {
        dc->VSSetShader(nullptr, nullptr, 0); 
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
    // return rendertarget for next pass?
    D3D11Core::Get().DeviceContext()->UpdateSubresource(m_camera->m_viewConstantBuffer.Get(), 0, nullptr, m_camera->GetCameraMatrixes(), 0, 0);
}
