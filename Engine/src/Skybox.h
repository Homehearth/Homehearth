#pragma once
#include "EnginePCH.h"


class Skybox
{
private:
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
	int nrOfIndices;
	ComPtr<ID3D11ShaderResourceView> m_skySrv;
	//ComPtr<ID3D11Texture2D> m_brdfLUT;
	std::shared_ptr<RTexture> m_brdfLUT;

	bool CreateVertIndBuffers();
	bool CreateTextureAndSRV(const std::string& fileName);

public:
	Skybox();
	virtual ~Skybox();

	bool Initialize(const std::string& fileName);

	void Render();
};