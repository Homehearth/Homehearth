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

    // OUTPUT MERGER.
    {
        DC->OMSetRenderTargets(1, PM->m_RTV_TextureEffectBlendMap.GetAddressOf(),        nullptr);
        DC->OMSetRenderTargets(1, PM->m_RTV_TextureEffectWaterEdgeMap.GetAddressOf(),    nullptr);
        DC->OMSetRenderTargets(1, PM->m_RTV_TextureEffectWaterFloorMap.GetAddressOf(),   nullptr);
        DC->OMSetRenderTargets(1, PM->m_RTV_TextureEffectWaterMap.GetAddressOf(),        nullptr);
        DC->OMSetRenderTargets(1, PM->m_RTV_TextureEffectWaterNormalMap.GetAddressOf(),  nullptr);
        //Might need to set view port
    }

    // DISPATCH
    {
        const int groupCount = static_cast<int>(ceil(m_MAX_PIXELS / 1024));
        DC->Dispatch(groupCount, 1, 1);
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
    
    pDeviceContext->ClearRenderTargetView(PM->m_RTV_TextureEffectBlendMap.Get(),        m_clearColor);
    pDeviceContext->ClearRenderTargetView(PM->m_RTV_TextureEffectWaterEdgeMap.Get(),    m_clearColor);
    pDeviceContext->ClearRenderTargetView(PM->m_RTV_TextureEffectWaterFloorMap.Get(),   m_clearColor);
    pDeviceContext->ClearRenderTargetView(PM->m_RTV_TextureEffectWaterMap.Get(),        m_clearColor);
    pDeviceContext->ClearRenderTargetView(PM->m_RTV_TextureEffectWaterNormalMap.Get(),  m_clearColor);
}

void TextureEffectPass::SetResources()
{
    m_WaterModel       = ResourceManager::Get().GetResource<RModel>("WaterMesh.obj");
    m_WaterEdgeModel   = ResourceManager::Get().GetResource<RModel>("WaterEdgeMesh.obj");
    m_WaterFloorModel  = ResourceManager::Get().GetResource<RModel>("WaterFloorModel.obj");

    m_WaterUV       = m_WaterModel.get()->GetTextureCoords();
    m_WaterEdgeUV   = m_WaterEdgeModel.get()->GetTextureCoords();
    m_WaterFloorUV  = m_WaterFloorModel.get()->GetTextureCoords();

    m_WaterAlbedoMap       = m_WaterModel.get()->GetTextures(ETextureType::albedo)[0];
    m_WaterNormalMap       = m_WaterModel.get()->GetTextures(ETextureType::normal)[0];
    m_WaterEdgeAlbedoMap   = m_WaterEdgeModel.get()->GetTextures(ETextureType::albedo)[0];
    m_WaterFloorAlbedoMap  = m_WaterFloorModel.get()->GetTextures(ETextureType::albedo)[0];

    PM->m_SRV_TextureEffectWaterEdgeMap    = m_WaterEdgeAlbedoMap.get()->GetShaderView();
    PM->m_SRV_TextureEffectWaterFloorMap   = m_WaterFloorAlbedoMap.get()->GetShaderView();
    PM->m_SRV_TextureEffectWaterMap        = m_WaterAlbedoMap.get()->GetShaderView();
    PM->m_SRV_TextureEffectWaterNormalMap  = m_WaterNormalMap.get()->GetShaderView();

}



