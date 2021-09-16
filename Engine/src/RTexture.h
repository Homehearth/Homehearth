#pragma once
#include "GResource.h"
// RTexture -> (RESOURCE)Texture

class RTexture : public resource::GResource
{
private:
	ID3D11Texture2D* m_texture = nullptr;

public:

	RTexture() {};
	~RTexture();

	// Inherited via GResource
	virtual bool Create(const std::string& filename) override;
};

class RBitMap : public resource::GResource
{
private:
	ID2D1Bitmap* m_texture = nullptr;

public:

	RBitMap() {};
	~RBitMap();
	// Inherited via GResource
	virtual bool Create(const std::string& filename) override;

};