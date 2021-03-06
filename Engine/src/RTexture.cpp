#include "EnginePCH.h"
#include "RTexture.h"

#define STB_IMAGE_IMPLEMENTATION
#pragma warning(push, 0)
#include <stb_image.h>
#pragma warning(pop)

RTexture::RTexture()
{
	m_format = ETextureChannelType::fourChannels;
	m_useMipmaps = true;
}

RTexture::RTexture(ETextureChannelType format, bool useMipmaps)
{
	m_format = format;
	m_useMipmaps = useMipmaps;
}

RTexture::~RTexture()
{ 
}

bool RTexture::StandardSetup(unsigned char* image, const UINT& width, const UINT& height)
{
	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = (void*)image;
	data.SysMemSlicePitch = 0;

	if (m_format == ETextureChannelType::oneChannel)
		data.SysMemPitch = static_cast<UINT>(width * 1);
	else
		data.SysMemPitch = static_cast<UINT>(width * 4);

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
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;

	if (m_format == ETextureChannelType::oneChannel)
		textureDesc.Format = DXGI_FORMAT_R8_UNORM;
	else
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	HRESULT hr = D3D11Core::Get().Device()->CreateTexture2D(&textureDesc, &data, &m_texture2D);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		LOG_WARNING("[Texture2D] Failed to create Texture2D!");
#endif
		return false;
	}

	hr = D3D11Core::Get().Device()->CreateShaderResourceView(m_texture2D.Get(), 0, &m_shaderView);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		LOG_WARNING("[Texture2D] Failed to create ShaderResourceView!");
#endif
		return false;
	}

	return true;
}

bool RTexture::GenerateMipMaps(unsigned char* image, const UINT& width, const UINT& height)
{
	//Setup the texturebuffer
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = (UINT)width;
	textureDesc.Height = (UINT)height;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;

	if (m_format == ETextureChannelType::oneChannel)
		textureDesc.Format = DXGI_FORMAT_R8_UNORM;
	else
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	HRESULT hr = D3D11Core::Get().Device()->CreateTexture2D(&textureDesc, nullptr, &m_texture2D);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		LOG_WARNING("[Texture2D] Failed to create Texture2D!");
#endif
		return false;
	}

	UINT mempitch = 1;
	if (m_format == ETextureChannelType::oneChannel)
		mempitch = width;
	else
		mempitch = width * 4;

	//Write in data to the first level
	D3D11Core::Get().DeviceContext()->UpdateSubresource(m_texture2D.Get(), 0u, nullptr, (void*)image, mempitch, 0u);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	hr = D3D11Core::Get().Device()->CreateShaderResourceView(m_texture2D.Get(), &srvDesc, &m_shaderView);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		LOG_WARNING("[Texture2D] Failed to create ShaderResourceView!");
#endif
		return false;
	}

	//Genarate mipmaps - will take som extra memory: 1/3 of each texture.
	D3D11Core::Get().DeviceContext()->GenerateMips(m_shaderView.Get());

	return true;
}

bool RTexture::Create(const std::string& filename)
{
	m_filename = filename;
	std::string filepath = TEXTUREPATH + filename;
	int width = 0;
	int height = 0;
	int comp = 0;
	unsigned char* image = nullptr;

	//Load in image
	if (m_format == ETextureChannelType::oneChannel)
		image = stbi_load(filepath.c_str(), &width, &height, &comp, STBI_grey);
	else
		image = stbi_load(filepath.c_str(), &width, &height, &comp, STBI_rgb_alpha);

	if (image == nullptr)
	{
#ifdef _DEBUG
		LOG_WARNING("[Texture] Failed to load image: %s", filepath.c_str());
#endif 
		return false;
	}

	if (m_useMipmaps)
	{
		if(width % 2 == 0 && height % 2 == 0)
		{
			if (!GenerateMipMaps(image, static_cast<UINT>(width), static_cast<UINT>(height)))
			{
				stbi_image_free(image);
				return false;
			}
		}
		else
		{
			if (!StandardSetup(image, static_cast<UINT>(width), static_cast<UINT>(height)))
			{
				stbi_image_free(image);
				return false;
			}
		}
	}
	else
	{
		if (!StandardSetup(image, static_cast<UINT>(width), static_cast<UINT>(height)))
		{
			stbi_image_free(image);
			return false;
		}
	}

	stbi_image_free(image);
	return true;
}

ID3D11ShaderResourceView*& RTexture::GetShaderView()
{
	return *m_shaderView.GetAddressOf();
}

ID3D11Texture2D*& RTexture::GetTexture2D()
{
	return *m_texture2D.GetAddressOf();
}

const std::string& RTexture::GetFilename() const
{
	return m_filename;
}

bool RTexture::DisableMipmaps()
{
	bool success = true;

	if (m_useMipmaps)
	{
		m_useMipmaps = false;
		if (!Create(m_filename))
			success = false;
	}
	return success;
}

bool RBitMap::Create(const std::string& filename)
{
	return D2D1Core::CreateImage(filename, &m_texture);
}

ID2D1Bitmap*& RBitMap::GetTexture()
{
	return *m_texture.GetAddressOf();
}
