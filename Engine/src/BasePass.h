#pragma once
#include "IRenderPass.h"

//--------------------------------------
// Forward Rendering, or GeometryPass.
//--------------------------------------
class BasePass : public IRenderPass
{
public:
	BasePass() = default;
	virtual ~BasePass() = default;

	void PreRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
	
	void Render(Scene* pScene) override;
	
	void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
};

