#pragma once
#include "IRenderPass.h"

class DepthPass : public IRenderPass
{
public:
	DepthPass() = default;
	virtual ~DepthPass() = default;

	void Initialize() override;

	void PreRender() override;

	void Render() override;

	void PostRender() override;

};

