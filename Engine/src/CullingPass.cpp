#include "EnginePCH.h"
#include "CullingPass.h"

void CullingPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
	ID3D11RenderTargetView* nullRTV[] = { nullptr };
	ID3D11DepthStencilView* nullDSV = { nullptr };

	// Setup.
	DC->OMSetRenderTargets(ARRAYSIZE(nullRTV), nullRTV, nullDSV);
	DC->OMSetDepthStencilState(PM->m_depthStencilStateLessEqual.Get(), 0);

	DC->VSSetShader(nullptr, nullptr, 0);
	DC->PSSetShader(nullptr, nullptr, 0);
	DC->CSSetShader(PM->m_computeFrustums.Get(), nullptr, 0);

	ID3D11Buffer* const buffers[] = {
		PM->m_screenToViewParamsCB.GetBuffer(),
		PM->m_dispatchParamsCB.GetBuffer()
	};
	DC->CSSetConstantBuffers(6, ARRAYSIZE(buffers), buffers); // ScreenToViewParamsCB + DispatchParamsCB
	DC->CSSetShaderResources(0, 1, PM->m_depthBufferSRV.GetAddressOf()); // DepthTexture.
	DC->CSSetShaderResources(11, 1, PM->m_frustums.srv.GetAddressOf()); // Frustums.
	
	// Update Lights.
	m_lights->Render(DC);
	PM->opaq_LightIndexCounter_data[0] = 0u;
	PM->trans_LightIndexCounter_data[0] = 0u;

	DC->CSSetUnorderedAccessViews(1, 1, PM->opaq_LightIndexCounter.uav.GetAddressOf(), nullptr);
	DC->CSSetUnorderedAccessViews(2, 1, PM->trans_LightIndexCounter.uav.GetAddressOf(), nullptr);
	DC->CSSetUnorderedAccessViews(3, 1, PM->opaq_LightIndexList.uav.GetAddressOf(), nullptr);
	DC->CSSetUnorderedAccessViews(4, 1, PM->trans_LightIndexList.uav.GetAddressOf(), nullptr);
	DC->CSSetUnorderedAccessViews(5, 1, PM->opaq_LightGrid.uav.GetAddressOf(), nullptr);
	DC->CSSetUnorderedAccessViews(6, 1, PM->trans_LightGrid.uav.GetAddressOf(), nullptr);
}

void CullingPass::Render(Scene* pScene)
{
	// Dispatch.
	GetDeviceContext()->Dispatch(PM->m_dispatchParams.numThreads.x, PM->m_dispatchParams.numThreads.y, 1);
}

void CullingPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
	// Cleanup.
	ID3D11UnorderedAccessView* nullUAV[] = { nullptr };
	DC->CSSetUnorderedAccessViews(1, ARRAYSIZE(nullUAV), nullUAV, nullptr);
	DC->CSSetUnorderedAccessViews(2, ARRAYSIZE(nullUAV), nullUAV, nullptr);
	DC->CSSetUnorderedAccessViews(3, ARRAYSIZE(nullUAV), nullUAV, nullptr);
	DC->CSSetUnorderedAccessViews(4, ARRAYSIZE(nullUAV), nullUAV, nullptr);
	DC->CSSetUnorderedAccessViews(5, ARRAYSIZE(nullUAV), nullUAV, nullptr);
	DC->CSSetUnorderedAccessViews(6, ARRAYSIZE(nullUAV), nullUAV, nullptr);

	ID3D11ShaderResourceView* nullSRV[] = { nullptr };
	GetDeviceContext()->CSSetShaderResources(0, ARRAYSIZE(nullSRV), nullSRV);
	GetDeviceContext()->CSSetShaderResources(11, ARRAYSIZE(nullSRV), nullSRV);
}
