#include "EnginePCH.h"
#include "TextureEffectPass.h"

#include "PipelineManager.h"
//#include "RMesh.h"

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
        dc->CSSetConstantBuffers(3, 1, pm->m_textureEffectConstantBuffer.GetAddressOf());
    }

    // SHADER RESOURCES.
    {
        dc->PSSetShaderResources(0, 0, nullptr);
        dc->VSSetShaderResources(0, 0, nullptr);
        dc->PSSetSamplers(1, 1, pm->m_pointSamplerState.GetAddressOf());
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
    m_camera = pScene->GetCurrentCamera();
}

void TextureEffectPass::PostRender()
{
    // Update constantbuffer here!
    //m_CBuffer.deltaTime  = *m_camera->GetDeltaTime();
    m_CBuffer.direction  = 45;
    m_CBuffer.radious    = 10;
    D3D11Core::Get().DeviceContext()->UpdateSubresource(m_pm->m_textureEffectConstantBuffer.Get(), 0, nullptr, &m_CBuffer, 0, 0);
}

void TextureEffectPass::CreateViews()
{
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    shaderResourceViewDesc.Format = textureDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;

    HRESULT result = DC->CreateShaderResourceView(normalData.Get(), &shaderResourceViewDesc, normalResourceView.GetAddressOf());
    if (FAILED(result)) { return false; };

}

