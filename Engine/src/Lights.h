#pragma once
#include "EnginePCH.h"

struct light_info_t
{
	dx::XMFLOAT4 nrOfLights;
};

class Lights
{
private:

	bool m_isInit = false;
	int m_currentLight = -1;

	std::vector<light_t> m_lights;
	ComPtr<ID3D11Buffer> m_lightBuffer;
	ComPtr<ID3D11ShaderResourceView> m_lightShaderView;
	ComPtr<ID3D11Buffer> m_lightInfoBuffer;

	const bool SetupLightBuffer();
	const bool SetupInfoBuffer();
	const bool UpdateLightBuffer();
	const bool UpdateInfoBuffer();


public:

	Lights();
	~Lights();

	bool Initialize();
	const bool IsInitialize() const;
	void Render(ID3D11DeviceContext* dc);
	void Add(const light_t& light);
	void EditLight(light_t L, const int& index = -1);

	void AddFromComp(entt::registry& reg, entt::entity ent);
};