#include "EnginePCH.h"
#include "RTexture.h"

static const bool CreateTexture(const std::string& filePath, ID3D11Texture2D** texture)
{
	// No path entered.
	if (filePath.length() <= 0)
		return false;

	D3D11_TEXTURE2D_DESC tDesc = {};
	tDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	tDesc.ArraySize = 1;
	tDesc.MipLevels = 1;
	tDesc.Usage = D3D11_USAGE_IMMUTABLE;
	tDesc.SampleDesc.Count = 1;
	tDesc.SampleDesc.Quality = 1;
	//tDesc.Width = 
	//tDesc.Height = 

	return true;
}

RTexture::RTexture(const std::string&& filePath)
{
	if (CreateTexture(filePath, &this->texture.texture2D))
	{
		this->active = 0;
	}
}

RTexture::RTexture(ID2D1Bitmap* texture)
{
	this->active = 1;
	this->texture.bitMap = texture;
}

RTexture::~RTexture()
{
	switch (this->active)
	{
	case 0:
		this->texture.texture2D->Release();
		break;
	case 1:
		this->texture.bitMap->Release();
		break;
	default:
		// nothing initalized
		break;
	}
}
