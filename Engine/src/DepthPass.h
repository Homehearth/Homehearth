#pragma once
#include "IRenderPass.h"

class DepthPass : public IRenderPass
{
public:
	DepthPass() = default;
	virtual ~DepthPass() = default;

	void Initialize() override;

	void PreRender(ID3D11DeviceContext* dc, PipelineManager* pm) override;

	void Render(Scene* pScene) override;

	void PostRender(ID3D11DeviceContext* dc, PipelineManager* pm) override;

};

