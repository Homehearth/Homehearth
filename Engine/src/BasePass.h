#pragma once
#include "IRenderPass.h"

//--------------------------------------
// Forward Rendering, or GeometryPass.
//--------------------------------------
class BasePass : public IRenderPass
{
private:
	bool m_isEnabled;

public:
	BasePass() = default;
	virtual ~BasePass() = default;
		
	void Initialize() override;

	bool IsEnabled() override { return m_isEnabled;	}
	
	void SetEnable(bool enable) override { m_isEnabled = enable; }
	
	void PreRender(ID3D11DeviceContext* dc, PipelineManager* pm) override;
	
	void Render() override;
	
	void PostRender() override;
	
};

