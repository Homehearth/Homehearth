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
	const bool UpdateInfoBuffer();

public:

	Lights();
	~Lights();

	bool Initialize();
	void Render();
};