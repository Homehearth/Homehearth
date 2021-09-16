#pragma once
#include "GResource.h"
// RTexture -> (RESOURCE)Texture

class RTexture : public resource::GResource
{
private:

	union Texture
	{
		ID3D11Texture2D* texture2D;
		ID2D1Bitmap* bitMap;
	} m_texture = {};
	// 0 -> Texture[0] : 1 -> Texture[1]
	int8_t m_active = -1;

public:

	RTexture();
	RTexture(const std::string&& filePath);
	RTexture(ID2D1Bitmap* texture);
	~RTexture();

	// Inherited via GResource
	virtual bool Create(const std::string& filename) override;
};