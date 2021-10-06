#pragma once
#include "IRenderPass.h"

class BasePass : public IRenderPass
{
public:
	BasePass() = default;
	virtual ~BasePass() = default;
		
	void Initialize() override;
	
	void PreRender() override;
	
	void Render() override;
	
	void PostRender() override;
	
};

