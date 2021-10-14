#pragma once
#include "GResource.h"
const UINT MAXSIZE = 4096;

enum class ETextureChannelType
{
	oneChannel,
	fourChannels
};

class RTexture : public resource::GResource
{
private:
	ETextureChannelType			m_format;
	ID3D11Texture2D*			m_texture;
	ID3D11ShaderResourceView*	m_shaderView;

public:
	RTexture();
	RTexture(ETextureChannelType format);
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
	ID2D1Bitmap*& GetTexture();
};
