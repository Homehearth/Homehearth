#pragma once
#include "IRenderPass.h"
#include "ShadowPass.h"
#include "DecalPass.h"

//--------------------------------------
// Forward Rendering, or GeometryPass.
//--------------------------------------
class BasePass : public IRenderPass
{
public:
	Skybox* m_skyboxRef = nullptr;
	ShadowPass* m_pShadowPass = nullptr;
	DecalPass* m_decalPass = nullptr;

	BasePass() = default;
	virtual ~BasePass() = default;

	void PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
	
	void Render(Scene* pScene) override;
	
	void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
};

