#pragma once
#include "RTexture.h"
#include "RMesh.h"

namespace objectPass
{
	enum class RenderPass {
		// Draws the object normally, Looks for texture, mesh and more.
		DEFAULT,

		// Draws the shadow for the object.
		SHADOW,
	};
}

class RenderableObject
{
private:

	// WHAT TO PUT HERE???

public:

	virtual ~RenderableObject() {};

	virtual void Render(const objectPass::RenderPass& pass = objectPass::RenderPass::DEFAULT) = 0;
};