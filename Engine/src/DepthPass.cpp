#include "EnginePCH.h"
#include "DepthPass.h"
#include "PipelineManager.h"
#include "RModel.h"

void DepthPass::PreRender()
{
	// Set DepthBuffer.
    ID3D11RenderTargetView* nullRTV[] = { nullptr };
    DC->OMSetRenderTargets(ARRAYSIZE(nullRTV), nullRTV, PM->m_depthStencilView.Get());
    DC->OMSetDepthStencilState(PM->m_depthStencilStateLessEqual.Get(), 0);

	// Set InputLayout, ConstantBuffers and Shaders.
	DC->IASetInputLayout(PM->m_defaultInputLayout.Get());
	
    DC->VSSetConstantBuffers(1, 1, CAMERA->m_viewConstantBuffer.GetAddressOf());
	
    DC->VSSetShader(PM->m_depthPassVertexShader.Get(), nullptr, 0);
    DC->PSSetShader(nullptr, nullptr, 0);
}

void DepthPass::Render(Scene* pScene)
{
   pScene->Render(); 
}

void DepthPass::PostRender()
{
	// Cleanup.
    ID3D11DepthStencilView* nullDSV = { nullptr };
    DC->OMSetRenderTargets(0, nullptr, nullDSV);
}