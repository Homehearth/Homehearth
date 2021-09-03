#include "EnginePCH.h"
#include "RTexture.h"

RTexture::RTexture(ID3D11Texture2D* texture)
{
	this->active = 0;
	this->texture.texture2D = texture;
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
