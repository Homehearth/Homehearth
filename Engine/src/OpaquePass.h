#pragma once
#include "IRenderPass.h"

class OpaquePass : public IRenderPass
{
public:
	OpaquePass() = default;
	virtual ~OpaquePass() = default;
	
	void Initialize() override;

	void PreRender(ID3D11DeviceContext* dc, PipelineManager* pm) override;

	void Render(Scene* pScene) override;

	void PostRender(ID3D11DeviceContext* dc, PipelineManager* pm) override;
};

