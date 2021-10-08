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

	void PreRender() override;
	
	void Render(Scene* pScene) override;
	
	void PostRender() override;
};

