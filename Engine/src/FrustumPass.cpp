#include "EnginePCH.h"
#include "FrustumPass.h"

void FrustumPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
	DC->VSSetShader(nullptr, nullptr, 0);
	DC->PSSetShader(nullptr, nullptr, 0);
	DC->CSSetShader(PM->m_computeFrustumsShader.Get(), nullptr, 0);

	ID3D11Buffer* const screenToView = PM->m_screenToViewParamsCB.GetBuffer();
	ID3D11Buffer* const dispatchParam = PM->m_dispatchParamsCB.GetBuffer();

	DC->CSSetConstantBuffers(11, 1, &screenToView); // ScreenToViewParamsCB
	DC->CSSetConstantBuffers(12, 1, &dispatchParam); //  DispatchParamsCB
	DC->CSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());  // Camera.
	DC->CSSetUnorderedAccessViews(0, 1, PM->m_frustums.uav.GetAddressOf(), nullptr); // Frustums
}

void FrustumPass::Render(Scene* pScene)
{
	PROFILE_FUNCTION();

	GetDeviceContext()->Dispatch(PM->m_dispatchParams.numThreadGroups.x, PM->m_dispatchParams.numThreadGroups.y, 1);
	
}

void FrustumPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    // Cleanup.
	ID3D11UnorderedAccessView* nullUAV[] = { nullptr };
	DC->CSSetShader(nullptr, nullptr, 0);
	DC->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);

    // Only run this pass once.
    SetEnable(true);
}
