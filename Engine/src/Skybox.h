#pragma once

class Skybox
{
private:
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
	ComPtr<ID3D11Buffer> m_constBuffer;
	int nrOfIndices;
	ComPtr<ID3D11ShaderResourceView> m_skySrv;
	ComPtr<ID3D11ShaderResourceView> m_radianceSrv;
	ComPtr<ID3D11ShaderResourceView> m_irradianceSrv;
	std::shared_ptr<RTexture> m_brdfLUT;


	sm::Vector3 m_tintColNight			= { 0.04f, 0.06f, 0.20f }; //Tint Color Night:		0.04f, 0.06f, 0.20f
	sm::Vector3 m_tintColDay			= { 1.00f, 1.00f, 1.00f }; //Tint Color Day:		1.00f, 1.00f, 1.00f
	sm::Vector3 m_tintColMorning		= { 0.95f, 0.65f, 0.15f }; //Tint Color Morning:	0.95f, 0.65f, 0.15f
	sm::Vector3 m_tintColEvening		= { 0.93f, 0.05f, 0.58f }; //Tint Color Evening:	0.93f, 0.05f, 0.58f
	sm::Vector3 m_tintCol				= {};	

	bool CreateVertIndBuffers();
	bool CreateTextureAndSRV(const std::string& fileName);
	bool CreateConstBuffer();

	void Update(ID3D11DeviceContext* dc);

public:
	Skybox();
	virtual ~Skybox();

	bool Initialize(const std::string& fileName);

	void Render();
	void Bind(ID3D11DeviceContext* dc);
};