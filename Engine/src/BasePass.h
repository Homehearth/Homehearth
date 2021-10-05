#pragma once
#include "IRenderPass.h"

class BasePass : public IRenderPass
{
public:
	BasePass() = default;
	virtual ~BasePass() = default;
		
	void Initialize() override;
	
	void PreRender(ID3D11DeviceContext* dc, PipelineManager* pm) override;
	
	void Render(Scene* pScene) override;
	
	void PostRender(ID3D11DeviceContext* dc, PipelineManager* pm) override;
	
};

