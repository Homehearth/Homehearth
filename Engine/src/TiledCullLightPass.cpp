#include "EnginePCH.h"
#include "TiledCullLightPass.h"

void TiledCullLightPass::Initialize()
{
}

void TiledCullLightPass::PreRender(ID3D11DeviceContext* dc, PipelineManager* pm)
{
    ID3D11ShaderResourceView* const nullSRV[] = { nullptr };
    ID3D11RenderTargetView* const nullRTV[] = { nullptr };
    ID3D11SamplerState* const nullSampler[] = { nullptr };
	
    dc->OMSetRenderTargets(1, nullRTV, nullptr);  // null color buffer and depth-stencil
    dc->VSSetShader(nullptr, nullptr, 0);   // null vertex shader
    dc->PSSetShader(nullptr, nullptr, 0);   // null pixel shader
    dc->PSSetShaderResources(0, 1, nullSRV);
    dc->PSSetShaderResources(1, 1, nullSRV);
    dc->PSSetSamplers(0, 1, nullSampler);
    //dc->CSSetShader(pLightCullCS, nullptr, 0);
    //dc->CSSetShaderResources(0, 1, g_Util.GetPointLightBufferCenterAndRadiusSRVParam());
    //dc->CSSetShaderResources(1, 1, g_Util.GetSpotLightBufferCenterAndRadiusSRVParam());
    //dc->CSSetShaderResources(2, 1, &pDepthSRV);
    //dc->CSSetUnorderedAccessViews(0, 1, g_Util.GetLightIndexBufferUAVParam(), nullptr);
    //dc->Dispatch(g_Util.GetNumTilesX(), g_Util.GetNumTilesY(), 1);
}

void TiledCullLightPass::Render(Scene* pScene)
{

}

void TiledCullLightPass::PostRender(ID3D11DeviceContext* dc, PipelineManager* pm)
{
	// Cleanup.
    ID3D11ShaderResourceView* const nullSRV[] = { nullptr };
    ID3D11UnorderedAccessView* const nullUAV[] = { nullptr };
	
    dc->CSSetShader(nullptr, nullptr, 0);
    dc->CSSetShaderResources(0, 1, nullSRV);
    dc->CSSetShaderResources(1, 1, nullSRV);
    dc->CSSetShaderResources(2, 1, nullSRV);
    dc->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
}

unsigned TiledCullLightPass::GetNumTilesX()
{
    return (unsigned)((m_width + TILE_RESOLUTION - 1) / (float)TILE_RESOLUTION);
}

unsigned TiledCullLightPass::GetNumTilesY()
{
    return (unsigned)((m_height + TILE_RESOLUTION - 1) / (float)TILE_RESOLUTION);
}

unsigned TiledCullLightPass::GetMaxNumLightsPerTile()
{
    return MAX_NUM_LIGHTS_PER_TILE;
}