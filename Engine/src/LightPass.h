#pragma once
#include "IRenderPass.h"


/*
 * The light culling pass of the Tiled Forward Shading technique uses a uniform grid of tiles to assign each active scene light to tiles in the grid.
 * This pass is usually executed using a compute shader which is invoked with one thread group for each tile in the grid.
 * 
 * 8x8 tiles = 8x8 thread groups (64 threads per thread group),
 * 16x16 tiles = 16x16 thread groups (256 threads per thread group),
 * 32x32 tiles = 32x32 thread groups (1,024 threads per group).
 *
 * Buffers: light list, light index list and light grid list.
 */
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

	void Render(Scene* pScene) override;

	void PostRender() override;
};

