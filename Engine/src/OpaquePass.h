#pragma once
#include "IRenderPass.h"

class OpaquePass : public IRenderPass
{
public:
	OpaquePass() = default;
	virtual ~OpaquePass() = default;
	
	void PreRender() override;

	void Render(Scene* pScene) override;

	void PostRender() override;
};

