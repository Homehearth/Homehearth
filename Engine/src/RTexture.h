#pragma once
#include <EnginePCH.h>
#include "GResource.h"
// RTexture -> (RESOURCE)Texture


class RTexture : public resource::GResource
{
private:

	union Texture
	{
		ID3D11Texture2D* texture2D;
		ID2D1Bitmap* bitMap;
	}texture = {};
	// 0 -> Texture[0] : 1 -> Texture[1]
	int8_t active = -1;

public:

	RTexture() {};
	RTexture(const std::string&& filePath);
	RTexture(ID2D1Bitmap* texture);
	~RTexture();
};