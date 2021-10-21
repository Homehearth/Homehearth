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
	unsigned char*				m_image;
	sm::Vector2					m_size;

	std::string					m_filepath;
	int							m_width;
	int							m_height;
	int							m_comp;
	bool						m_isFree = true;

public:
	RTexture();
	RTexture(ETextureChannelType format);
	~RTexture();

	// Inherited via GResource
	virtual bool Create(const std::string& filename) override;
	ID3D11ShaderResourceView*& GetShaderView();
	unsigned char* GetImageData();
	sm::Vector2 GetSize();
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
