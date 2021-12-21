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

    PM->m_dispatchParamsCB.SetData(pDeviceContext, PM->m_dispatchParams);
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
    SetEnable(false);

 //   const auto screenWidth = static_cast<uint32_t>(max(PM->m_viewport.Width, 1u));
 //   const auto screenHeight = static_cast<uint32_t>(max(PM->m_viewport.Height, 1u));

 //   // We will need 1 frustum for each grid cell.
 //   // [x, y] screen resolution and [z, z] tile size yield [x/z, y/z] grid size.
 //   // Resulting in a total of x/z * y/z frustums.
 //   constexpr uint32_t TILE_SIZE = 16u;

 //   //
 //   // Update DispatchParams.
 //   //

 //   const dx::XMUINT4 numThreadGroups = {
 //       (uint32_t)std::ceil((float)screenWidth / (float)TILE_SIZE),
 //       (uint32_t)std::ceil((float)screenHeight / (float)TILE_SIZE),
 //       1u,
 //       1u
 //   };

	//const dx::XMUINT4 numThreads = {
	//	screenWidth,
	//	screenHeight,
	//	1u,
	//	1u
	//};

 //  PM->m_dispatchParams.numThreadGroups = numThreadGroups;
 //  PM->m_dispatchParams.numThreads = numThreads;
 //  PM->m_dispatchParamsCB.SetData(pDeviceContext, PM->m_dispatchParams);
}
