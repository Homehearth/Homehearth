#pragma once
#include "GResource.h"

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
	ComPtr<ID3D11ShaderResourceView>	m_shaderView;
	ComPtr<ID3D11Texture2D>				m_texture2D;
	bool StandardSetup(unsigned char* image, const UINT& width, const UINT& height);
	bool GenerateMipMaps(unsigned char* image, const UINT& width, const UINT& height);

public:
	RTexture();
	RTexture(ETextureChannelType format);
	~RTexture();

	// Inherited via GResource
	virtual bool Create(const std::string& filename) override;
	ID3D11ShaderResourceView*& GetShaderView();
	ID3D11Texture2D*& GetTexture2D();
};

class RBitMap : public resource::GResource
{
private:
	ComPtr<ID2D1Bitmap> m_texture;

public:
	RBitMap() = default;
	~RBitMap() = default;
	// Inherited via GResource
	virtual bool Create(const std::string& filename) override;
	ID2D1Bitmap*& GetTexture();
};
