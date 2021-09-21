#pragma once
#include "IRenderPass.h"

class LightPass : public IRenderPass
{
private:
	bool m_isEnabled;

public:
	LightPass() = default;
	virtual ~LightPass() = default;
	
	void Initialize() override;

	bool IsEnabled() override { return m_isEnabled; }

	void SetEnable(bool enable) override { m_isEnabled = enable; }

	void PreRender(ID3D11DeviceContext* dc, PipelineManager* pm) override;

	void Render() override;

	void PostRender() override;
};

