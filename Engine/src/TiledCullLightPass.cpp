#include "EnginePCH.h"
#include "TiledCullLightPass.h"

void TiledCullLightPass::Initialize()
{
}

void TiledCullLightPass::PreRender()
{
    ClearPipelineSettings();
	
    auto dc = GetContext();	
	
    //dc->CSSetShader(pLightCullCS, nullptr, 0);
    //dc->CSSetShaderResources(0, 1, g_Util.GetPointLightBufferCenterAndRadiusSRVParam());
    //dc->CSSetShaderResources(1, 1, g_Util.GetSpotLightBufferCenterAndRadiusSRVParam());
    //dc->CSSetShaderResources(2, 1, &pDepthSRV);
    //dc->CSSetUnorderedAccessViews(0, 1, g_Util.GetLightIndexBufferUAVParam(), nullptr);
    dc->Dispatch(GetNumTilesX(), GetNumTilesY(), 1);
}

void TiledCullLightPass::Render()
{

}

void TiledCullLightPass::PostRender()
{	


}

unsigned TiledCullLightPass::GetNumTilesX()
{
    return (unsigned)((m_width + TILE_SIZE - 1) / (float)TILE_SIZE);
}

unsigned TiledCullLightPass::GetNumTilesY()
{
    return (unsigned)((m_height + TILE_SIZE - 1) / (float)TILE_SIZE);
}

unsigned TiledCullLightPass::GetMaxNumLightsPerTile()
{
    return MAX_NUM_LIGHTS_PER_TILE;
}