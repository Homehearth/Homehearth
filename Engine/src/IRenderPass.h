#pragma once
#include "PipelineManager.h"
#include "Lights.h"

//--------------------------------------
// Abstract class.
//--------------------------------------
class IRenderPass
{
protected:
	Lights* m_lights;
public:
	virtual ~IRenderPass() = default;
	virtual void Initialize() = 0;
	virtual bool IsEnabled() = 0;
	virtual void SetEnable(bool enable) = 0;
	virtual void PreRender(ID3D11DeviceContext* dc, PipelineManager * pm) = 0;
	virtual void Render(Scene* pScene) = 0;
	virtual void PostRender() = 0;
	virtual void SetLights(Lights* light);
};