#pragma once
#include "GResource.h"
// RTexture -> (RESOURCE)Texture

class RTexture : public resource::GResource
{
private:
	ID3D11Texture2D* m_texture = nullptr;
	ID3D11ShaderResourceView* m_shaderView = nullptr;

public:

	RTexture() {};
	~RTexture();

	// Inherited via GResource
	virtual bool Create(const std::string& filename) override;
	ID3D11ShaderResourceView*& GetShaderView();
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