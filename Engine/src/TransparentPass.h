#pragma once
#include "IRenderPass.h"

class TransparentPass : public IRenderPass
{
private:
	bool m_isEnabled;

public:
	TransparentPass() = default;
	virtual ~TransparentPass() = default;
	
	void Initialize() override;

	bool IsEnabled() override { return m_isEnabled; }

	void SetEnable(bool enable) override { m_isEnabled = enable; }

	void PreRender(ID3D11DeviceContext* dc, PipelineManager* pm) override;

	void Render(Scene* pScene) override;

	void PostRender() override;
};

