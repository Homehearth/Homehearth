#include "EnginePCH.h"
#include "CullingPass.h"

void CullingPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{

    // todo: LightCullingPass
    // device->CreateUnorderedAccessView(tex2D, &uavDesc, &o_LightGrid_tex);
    // device->CreateUnorderedAccessView(tex2D2, &uavDesc, &t_LightGrid_tex);
    // device->CreateShaderResourceView(tex2D, &srvDesc, &o_LightGrid_texSRV);
    // device->CreateShaderResourceView(tex2D, &srvDesc, &t_LightGrid_texSRV);
}

void CullingPass::Render(Scene* pScene)
{
    //SetCullBack(true);
    // render opaq.
    //SetCullBack(false);
	//// render trans.
    //SetCullBack(true);
}

void CullingPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{

}
