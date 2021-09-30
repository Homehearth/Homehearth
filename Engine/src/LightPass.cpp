#include "EnginePCH.h"
#include "LightPass.h"

void LightPass::Initialize()
{
	// Initialize Pass.
	// accept scene.
	// accept materials.
	// accept 
}

void LightPass::PreRender(ID3D11DeviceContext* dc, PipelineManager* pm)
{
// Require: L is a list of n lights.
// Require : C is the current index in the global light index list.
// Require : I is the global light index list.
// Require : G is the 2D grid storing the index and light count into the global
// light index list.
// Require : tid is the 2D index of the current thread within the dispatch.
// Require : B is the 2D size of a tile.
}

void LightPass::Render(Scene* pScene)
{
	// Render objects.
}

void LightPass::PostRender()
{
	// ?
}