#pragma once
#include "PipelineManager.h"

//--------------------------------------
// Abstract class.
//--------------------------------------
class IRenderPass
{
public:
	virtual ~IRenderPass() = default;
	virtual void Initialize() = 0;
	virtual bool IsEnabled() = 0;
	virtual void SetEnable(bool enable) = 0;
	virtual void PreRender(ID3D11DeviceContext* dc, PipelineManager * pm) = 0;
	virtual void Render(Scene* pScene) = 0;
	virtual void PostRender(ID3D11DeviceContext* dc = D3D11Core::Get().DeviceContext()) = 0;
};