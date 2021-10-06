#pragma once
#include "IRenderPass.h"

class OpaquePass : public IRenderPass
{
public:
	OpaquePass() = default;
	virtual ~OpaquePass() = default;
	
	void Initialize() override;

	void PreRender() override;

	void Render() override;

	void PostRender() override;
};

