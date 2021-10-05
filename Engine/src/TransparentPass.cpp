#include "EnginePCH.h"
#include "TransparentPass.h"

void TransparentPass::Initialize()
{
}

void TransparentPass::PreRender(ID3D11DeviceContext* dc, PipelineManager* pm)
{
	// Bind Camera and PipelineState.
}

void TransparentPass::Render(Scene* pScene)
{
	// Render objects.
}

void TransparentPass::PostRender(ID3D11DeviceContext* dc, PipelineManager* pm)
{
	// Cleanup.
}