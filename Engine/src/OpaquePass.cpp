#include "EnginePCH.h"
#include "OpaquePass.h"

void OpaquePass::Initialize()
{
	// Initialize Pass.
	// accept scene.
	// accept materials.
	// accept 
}

void OpaquePass::PreRender(ID3D11DeviceContext* dc, PipelineManager* pm)
{
	// Bind Camera and PipelineState.
}

void OpaquePass::Render(Scene* pScene)
{
	// Render objects.
}

void OpaquePass::PostRender()
{
	// ?
}