#pragma once
#include "IRenderPass.h"

class TransparentPass : public IRenderPass
{
public:
	TransparentPass() = default;
	virtual ~TransparentPass() = default;
	
	void PreRender() override;

	void Render(Scene* pScene) override;

	void PostRender() override;
};

