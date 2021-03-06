#include "EnginePCH.h"
#include "Skybox.h"
#include <DDSTextureLoader.h>

#define DV D3D11Core::Get().Device()
#define DCSB D3D11Core::Get().DeviceContext()

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
	std::string path = "../Assets/Skybox/";

	std::string sky = path + "sky_" + fileName;

	std::wstring pathWS = std::wstring(sky.begin(), sky.end());
	HRESULT hr = dx::CreateDDSTextureFromFile(DV, pathWS.c_str(), nullptr, m_skySrv.GetAddressOf());
	if (FAILED(hr))
		return false;
	
	std::string radiance = path + "radiance_" + fileName;
	pathWS = std::wstring(radiance.begin(), radiance.end());
	hr = dx::CreateDDSTextureFromFile(DV, pathWS.c_str(), nullptr, m_radianceSrv.GetAddressOf());
	if (FAILED(hr))
		return false;

	std::string irradiance = path + "irradiance_" + fileName;
	pathWS = std::wstring(irradiance.begin(), irradiance.end());
	hr = dx::CreateDDSTextureFromFile(DV, pathWS.c_str(), nullptr, m_irradianceSrv.GetAddressOf());
	if (FAILED(hr))
		return false;

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

	return true;
}

bool Skybox::CreateConstBuffer()
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeof(sm::Vector4);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = &m_tintCol;

	HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&desc, &data, m_constBuffer.GetAddressOf());

	return !FAILED(hr);
}

void Skybox::Update(ID3D11DeviceContext* dc = DCSB)
{

	if (m_currentTime > MORNING && m_currentTime < DAY)
		this->Tint(m_currentTime, MORNING, DAY, m_tintColMorning, m_tintColDay);

	else if (m_currentTime > MID_DAY && m_currentTime < EVENING)
		this->Tint(m_currentTime, MID_DAY, EVENING, m_tintColDay, m_tintColEvening);

	else if (m_currentTime > EVENING && m_currentTime < NIGHT)
		this->Tint(m_currentTime, EVENING, NIGHT, m_tintColEvening, m_tintColNight);

	else if (m_currentTime > EARLY_MORNING)
		this->Tint(m_currentTime, EARLY_MORNING, 1.0f, m_tintColNight, m_tintColMorning);


	dc->UpdateSubresource(m_constBuffer.Get(), 0, nullptr, &m_tintCol, 0, 0);
}

void Skybox::Tint(float currentTime, float startTime, float endTime, sm::Vector3 startColor, sm::Vector3 endColor)
{
	float diff = endTime - startTime;
	float progress = (currentTime - startTime) / diff;
	m_tintCol = util::Lerp(startColor, endColor, progress);
}

Skybox::Skybox()
{
	nrOfIndices = 0;
	m_currentTime = 0.f;
	m_tintCol = m_tintColMorning;
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
	if (!CreateConstBuffer())
		return false;

	return true;
}

void Skybox::Render()
{
	UINT stride = sizeof(sm::Vector3);
	UINT offset = 0;
	DCSB->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	DCSB->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, offset);

	Bind(DCSB);

	DCSB->DrawIndexed(nrOfIndices, 0, 0);
}

void Skybox::Bind(ID3D11DeviceContext* dc = DCSB)
{
	dc->PSSetShaderResources(96, 1, m_radianceSrv.GetAddressOf());
	dc->PSSetShaderResources(97, 1, m_irradianceSrv.GetAddressOf());
	dc->PSSetShaderResources(98, 1, m_skySrv.GetAddressOf());
	dc->PSSetShaderResources(99, 1, &m_brdfLUT.get()->GetShaderView());
	dc->PSSetConstantBuffers(13, 1, m_constBuffer.GetAddressOf());

	Update(dc);
}

void Skybox::UpdateTime(float pTime)
{
	m_currentTime = pTime;
}
