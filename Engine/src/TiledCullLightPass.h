#pragma once
#include "IRenderPass.h"


/*
 * The light culling pass of the Tiled Forward Shading technique uses a uniform grid of tiles to assign each active scene light to tiles in the grid.
 * This pass is usually executed using a compute shader which is invoked with one thread group for each tile in the grid.
 * The maximum depth value in the tile is used to define the far clipping plane and
 * the minimum depth value in the tile is used to define the near clipping plane for the tile’s view space frustum.
 * 
 * 8x8 tiles = 8x8 thread groups (64 threads per thread group),
 * 16x16 tiles = 16x16 thread groups (256 threads per thread group),
 * 32x32 tiles = 32x32 thread groups (1,024 threads per group).
 *
 * Buffers: light list, light index list and light grid list.
 *
 * For point lights, the radius of the bounding sphere for the light (used for culling) 
 * and the falloff distance of the light (used for lighting) are the same.
 */

class TiledCullLightPass : public IRenderPass
{
private:
	static const unsigned TILE_RESOLUTION = 8;
	static const unsigned MAX_NUM_LIGHTS_PER_TILE = 255;

	// RenderTargetView.
	unsigned m_width;
	unsigned m_height;
	
	unsigned GetNumTilesX();
	unsigned GetNumTilesY();
	unsigned GetMaxNumLightsPerTile();

public:
	TiledCullLightPass() = default;
	virtual ~TiledCullLightPass() = default;
	
	void Initialize() override;

	void PreRender(ID3D11DeviceContext* dc, PipelineManager* pm) override;

	void Render(Scene* pScene) override;

	void PostRender(ID3D11DeviceContext* dc, PipelineManager* pm) override;
};

