#pragma once
#include "IRenderPass.h"

//--------------------------------------
// Forward Rendering, or GeometryPass.
//--------------------------------------
class AnimationPass : public IRenderPass
{
public:
	AnimationPass() = default;
	virtual ~AnimationPass() = default;

	void PreRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

	void Render(Scene* pScene) override;

	void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
};
