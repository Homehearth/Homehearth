#pragma once
#include "EnginePCH.h"

struct light_info_t
{
	dx::XMFLOAT4 nrOfLights;
};

class Lights
{
private:

	std::vector<light_t> m_lights;
	ComPtr<ID3D11Buffer> m_lightBuffer;
	ComPtr<ID3D11ShaderResourceView> m_lightShaderView;
	ComPtr<ID3D11Buffer> m_lightInfoBuffer;

	const bool SetupLightBuffer();
	const bool SetupInfoBuffer();
	const bool UpdateLightBuffer();
	const bool UpdateInfoBuffer();

	bool m_isInit = false;

public:

	Lights();
	~Lights();

	bool Initialize();
	const bool IsInitialize() const;
	void Render(ID3D11DeviceContext* dc);
	void Add(const light_t& light);
};