#include "EnginePCH.h"
#include "RTexture.h"

#define STB_IMAGE_IMPLEMENTATION
#pragma warning(push, 0)
#include <stb_image.h>
#pragma warning(pop)

RTexture::~RTexture()
{
	if (m_texture)
		m_texture->Release();
	if (m_shaderView)
		m_shaderView->Release();
}

bool RTexture::Create(const std::string& filename)
{
	std::string filepath = "../Assets/Textures/" + filename;
	int width;
	int height;
	int comp;

	//Load in image
	unsigned char* image = stbi_load(filepath.c_str(), &width, &height, &comp, STBI_rgb_alpha);
	if (!image)
	{
#ifdef _DEBUG
		LOG_WARNING("[Texture] Failed to load image: %s", filepath.c_str());
#endif 
		stbi_image_free(image);
		return false;
	}

	//The texture is to large for the engine
	if (width > MAXSIZE || height > MAXSIZE)
	{
#ifdef _DEBUG
		LOG_WARNING("[Texture] %s is too large (%d x %d)", filename.c_str(), width, height);
#endif
		stbi_image_free(image);
		return false;
	}

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = (void*)image;
	data.SysMemPitch = static_cast<UINT>(width * 4);
	data.SysMemSlicePitch = 0;

	//Setup the texturebuffer
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = (UINT)width;
	textureDesc.Height = (UINT)height;
	textureDesc.MiscFlags = 0;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
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
	return false;
}