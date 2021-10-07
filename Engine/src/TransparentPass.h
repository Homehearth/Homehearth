#pragma once
#include "IRenderPass.h"

class TransparentPass : public IRenderPass
{
public:
	TransparentPass() = default;
	virtual ~TransparentPass() = default;
	
	void Initialize() override;

	void PreRender(ID3D11DeviceContext* dc, PipelineManager* pm) override;

	void Render(Scene* pScene) override;

	void PostRender(ID3D11DeviceContext* dc, PipelineManager* pm) override;
};

