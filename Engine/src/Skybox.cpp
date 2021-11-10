#include "EnginePCH.h"
#include "Skybox.h"
#include <DDSTextureLoader.h>

#define DC D3D11Core::Get().DeviceContext()

bool Skybox::CreateVertIndBuffers()
{
	float scale = 1.0f;
	sm::Vector3 vertices[] =
	{
		//Frontside
		{-scale,  scale,  scale},	//Top left		0
		{ scale,  scale,  scale},	//Top right		1
		{-scale, -scale,  scale},	//Down left		2
		{ scale, -scale,  scale},	//Down right	3
		//Backside
		{-scale,  scale, -scale},	//Top left		4
		{ scale,  scale, -scale},	//Top right		5
		{-scale, -scale, -scale},	//Down left		6
		{ scale, -scale, -scale}	//Down right	7
	};

	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = sizeof(vertices);
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.CPUAccessFlags = 0;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = vertices;

	HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&desc, &data, m_vertexBuffer.GetAddressOf());
	if(FAILED(hr))
		return false;

	uint16_t indices[] = 
	{
		//Front
		0, 1, 3,
		3, 2, 0,
		//Back
		5, 4, 6,
		6, 7, 5,
		//Left
		4, 0, 2,
		2, 6, 4,
		//Right
		1, 5, 7,
		7, 3, 1,
		//Top
		4, 5, 1,
		1, 0, 4,
		//Bottom
		2, 3, 7,
		7, 6, 2
	};

	nrOfIndices = sizeof(indices) / sizeof(uint16_t);
	desc.ByteWidth = sizeof(indices);
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	data.pSysMem = indices;

	hr = D3D11Core::Get().Device()->CreateBuffer(&desc, &data, m_indexBuffer.GetAddressOf());

	return !FAILED(hr);
}

bool Skybox::CreateTextureAndSRV(const std::string& fileName)
{
	std::string fileNameLong = "../Assets/Skybox/" + fileName;
	std::wstring fileNameLongWS = std::wstring(fileNameLong.begin(), fileNameLong.end());
	HRESULT hr = dx::CreateDDSTextureFromFile(D3D11Core::Get().Device(), DC, fileNameLongWS.c_str(), nullptr, m_skySrv.GetAddressOf());
	

	std::string lut = "ibl_brdf_lut.png";

	std::shared_ptr<RTexture> texture = ResourceManager::Get().GetResource<RTexture>(lut, false);
	m_brdfLUT = ResourceManager::Get().GetResource<RTexture>(lut, false);
	if (!m_brdfLUT)
	{
		m_brdfLUT = std::make_shared<RTexture>();

		if (!m_brdfLUT->Create(lut))
			return false;

		ResourceManager::Get().AddResource(lut, texture);
	}

	return !FAILED(hr);
}

Skybox::Skybox()
{
	nrOfIndices = 0;
}

Skybox::~Skybox()
{
}

bool Skybox::Initialize(const std::string& fileName)
{
	if (!CreateVertIndBuffers())
		return false;
	if (!CreateTextureAndSRV(fileName))
		return false;

	return true;
}

void Skybox::Render()
{
	UINT stride = sizeof(sm::Vector3);
	UINT offset = 0;
	DC->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	DC->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, offset);
	DC->PSSetShaderResources(15, 1, m_skySrv.GetAddressOf());
	DC->PSSetShaderResources(99, 1, &m_brdfLUT.get()->GetShaderView());

	DC->DrawIndexed(nrOfIndices, 0, 0);
}
