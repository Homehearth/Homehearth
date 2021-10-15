#pragma once
#pragma once
#include "IRenderPass.h"

//--------------------------------------
// Forward Rendering, or GeometryPass.
//--------------------------------------
class TextureEffectPass : public IRenderPass
{
private:
	bool m_isEnabled;
	Camera* m_camera;
	texture_effect_t m_CBuffer;
	PipelineManager *m_pm;

public:
	TextureEffectPass() = default;
	virtual ~TextureEffectPass() = default;

	void Initialize() override;

	bool IsEnabled() override { return m_isEnabled; }

	void SetEnable(bool enable) override { m_isEnabled = enable; }

	void PreRender(ID3D11DeviceContext* dc, PipelineManager* pm) override;

	void Render(Scene* pScene) override;

	void PostRender() override;

	void CreateViews();
};

