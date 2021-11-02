#include "EnginePCH.h"
#include "RTexture.h"

#define STB_IMAGE_IMPLEMENTATION
#pragma warning(push, 0)
#include <stb_image.h>
#pragma warning(pop)

RTexture::RTexture()
{
	m_format = ETextureChannelType::fourChannels;
	m_texture = nullptr;
	m_shaderView = nullptr;
}

RTexture::RTexture(ETextureChannelType format)
{
	m_format = format;
	m_texture = nullptr;
	m_shaderView = nullptr;
	m_image = nullptr;
}

RTexture::~RTexture()
{
	if (m_texture)
		m_texture->Release();
	if (m_shaderView)
		m_shaderView->Release();
	if (!m_isFree)					//Is to free only when getImageData was called
		stbi_image_free(m_image);
}

sm::Vector2 RTexture::GetSize()
{
	return m_size;
}

unsigned char* RTexture::GetImageData(std::string fileName)
{
	m_filepath = TEXTUREPATH + fileName;
	m_width = 0;
	m_height = 0;
	m_comp = 0;
	unsigned char* image = nullptr;

	if (m_format == ETextureChannelType::oneChannel)
		image = stbi_load(m_filepath.c_str(), &m_width, &m_height, &m_comp, STBI_grey);
	else
		image = stbi_load(m_filepath.c_str(), &m_width, &m_height, &m_comp, STBI_rgb_alpha);

	if (image == nullptr)
	{
#ifdef _DEBUG
		LOG_WARNING("[Texture] Failed to load image: %s", m_filepath.c_str());
#endif 
	}

	m_size = { (float)m_width, (float)m_height };
	m_isFree = false;
	return m_image;
}

bool RTexture::Create(const std::string& filename)
{
	m_filepath = TEXTUREPATH + filename;
	m_width = 0;
	m_height = 0;
	m_comp = 0;
	unsigned char* image = nullptr;

	//Load in image
	if (m_format == ETextureChannelType::oneChannel)
		image = stbi_load(m_filepath.c_str(), &m_width, &m_height, &m_comp, STBI_grey);
	else
		image = stbi_load(m_filepath.c_str(), &m_width, &m_height, &m_comp, STBI_rgb_alpha);

	if (image == nullptr)
	{
#ifdef _DEBUG
		LOG_WARNING("[Texture] Failed to load image: %s", m_filepath.c_str());
#endif 
		return false;
	}

	//The texture is to large for the engine
	if (m_width > MAXSIZE || m_height > MAXSIZE)
	{
#ifdef _DEBUG
		LOG_WARNING("[Texture] %s is too large (%d x %d)", filename.c_str(), m_width, m_height);
#endif
		stbi_image_free(image);
		return false;
	}
	m_size = { (float)m_width, (float)m_height };

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = (void*)image;
	data.SysMemSlicePitch = 0;

	if (m_format == ETextureChannelType::oneChannel)
		data.SysMemPitch = static_cast<UINT>(m_width * 1);
	else if (m_format == ETextureChannelType::fourChannels)
		data.SysMemPitch = static_cast<UINT>(m_width * 4);

	//Setup the texturebuffer
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = (UINT)m_width;
	textureDesc.Height = (UINT)m_height;
	textureDesc.MiscFlags = 0;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;	//D3D11_BIND_RENDER_TARGET
	textureDesc.CPUAccessFlags = 0;

	if (m_format == ETextureChannelType::oneChannel)
		textureDesc.Format = DXGI_FORMAT_R8_UNORM;
	else if (m_format == ETextureChannelType::fourChannels)
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	HRESULT hr = D3D11Core::Get().Device()->CreateTexture2D(&textureDesc, &data, &m_texture);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		LOG_WARNING("[Texture2D] Failed to create Texture2D!");
#endif
		stbi_image_free(image);
		return false;
	}

	hr = D3D11Core::Get().Device()->CreateShaderResourceView(m_texture, 0, &m_shaderView);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		LOG_WARNING("[Texture2D] Failed to create ShaderResourceView!");
#endif
		stbi_image_free(image);
		return false;
	}

	stbi_image_free(image);
	return true;
}

ID3D11ShaderResourceView*& RTexture::GetShaderView()
{
	return m_shaderView;
}

RBitMap::~RBitMap()
{
	if (m_texture)
		m_texture->Release();
}

bool RBitMap::Create(const std::string& filename)
{
	return D2D1Core::CreateImage(filename, &m_texture);
}

ID2D1Bitmap*& RBitMap::GetTexture()
{
	return m_texture;
}
