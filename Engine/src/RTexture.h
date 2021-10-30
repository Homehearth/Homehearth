#pragma once
#include "GResource.h"
const UINT MAXSIZE = 4096;

/*
	Can genarate mipmaps for textures to avoid flickering on textures far away.
	Takes 1/3 extra memory per texture.
*/

enum class ETextureChannelType
{
	oneChannel,
	fourChannels
};

class RTexture : public resource::GResource
{
private:
	ETextureChannelType					m_format;
	ComPtr<ID3D11Texture2D>				m_texture;
	ComPtr<ID3D11ShaderResourceView>	m_shaderView;

public:
	RTexture();
	RTexture(ETextureChannelType format);
	~RTexture();

	bool StandardSetup(unsigned char* image, const UINT& width, const UINT& height);
	bool GenerateMipMaps(unsigned char* image, const UINT& width, const UINT& height);

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
