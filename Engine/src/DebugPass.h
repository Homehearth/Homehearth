#pragma once
#include "IRenderPass.h"

//--------------------------------------
// Forward Rendering, or GeometryPass.
//--------------------------------------
class DebugPass : public IRenderPass
{
private:
	bool m_isEnabled = false;
	Camera* m_camera = nullptr;

public:
	DebugPass() = default;
	virtual ~DebugPass() = default;

	void Initialize() override;

	bool IsEnabled() override { return m_isEnabled; }

	void SetEnable(bool enable) override { m_isEnabled = enable; }

	void PreRender(ID3D11DeviceContext* dc, PipelineManager* pm) override;

	void Render(Scene* pScene) override;

	void PostRender() override;

	void SetCamera(Camera* camera);
	bool HasCamera();

};
