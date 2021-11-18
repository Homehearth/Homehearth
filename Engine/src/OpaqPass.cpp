#include "EnginePCH.h"
#include "OpaqPass.h"

void OpaqPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
	DC->IASetInputLayout(PM->m_defaultInputLayout.Get());

	DC->VSSetShader(PM->m_defaultVertexShader.Get(), nullptr, 0);
	DC->PSSetShader(PM->m_defaultPixelShader.Get(), nullptr, 0);

	DC->VSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());
	DC->PSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());

	DC->PSSetSamplers(0, 1, PM->m_pointSamplerState.GetAddressOf());
	DC->PSSetSamplers(1, 1, PM->m_linearSamplerState.GetAddressOf());
	DC->PSSetSamplers(2, 1, PM->m_anisotropicSamplerState.GetAddressOf());
	DC->PSSetSamplers(3, 1, PM->m_cubemapSamplerState.GetAddressOf());

	DC->RSSetViewports(1, &PM->m_viewport);
	DC->RSSetState(PM->m_rasterState.Get());

	DC->PSSetShaderResources(17, 1, PM->opaq_LightIndexList.srv.GetAddressOf());
	DC->PSSetShaderResources(18, 1, PM->opaq_LightGrid.srv.GetAddressOf());
	
	DC->OMSetRenderTargets(1, PM->m_backBuffer.GetAddressOf(), PM->m_depthStencilView.Get());
	DC->OMSetDepthStencilState(PM->m_depthStencilStateLessEqual.Get(), 0);
}

void OpaqPass::Render(Scene* pScene)
{
	// Opaque stuff.
	pScene->Render();
}

void OpaqPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
	ID3D11ShaderResourceView* nullSRV[] = { nullptr };
	DC->PSSetShaderResources(17, 1, nullSRV);
	DC->PSSetShaderResources(18, 1, nullSRV);
}
