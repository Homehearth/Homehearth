#pragma once
#include "RTexture.h"
#include "RMesh.h"

class RenderableObject
{
private:

	// WHAT TO PUT HERE???

public:

	virtual ~RenderableObject() {};

	virtual void Render() = 0;
};