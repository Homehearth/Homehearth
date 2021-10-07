#pragma once
#include "IRenderPass.h"

/*
 * Write the depth values of the scene from the camera's perspective into a depth buffer.
 */
class DepthPass : public IRenderPass
{
private:
	bool m_isEnabled;

public:
	DepthPass() = default;
	virtual ~DepthPass() = default;

	void Initialize() override;

	bool IsEnabled() override { return m_isEnabled; }

	void SetEnable(bool enable) override { m_isEnabled = enable; }

	void PreRender(ID3D11DeviceContext* dc, PipelineManager* pm) override;

	void Render(Scene* pScene) override;

	void PostRender(ID3D11DeviceContext* dc = D3D11Core::Get().DeviceContext()) override;

};

