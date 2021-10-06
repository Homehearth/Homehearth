#pragma once
#include "IRenderPass.h"

class TransparentPass : public IRenderPass
{
public:
	TransparentPass() = default;
	virtual ~TransparentPass() = default;
	
	void Initialize() override;

	void PreRender() override;

	void Render() override;

	void PostRender() override;
};

