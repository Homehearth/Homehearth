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

	//Create a structured buffer to use in pixelshader with all the lights
	const bool SetupLightBuffer();
	//Create a constant buffer to use in pixelshader with info on how many lights there are
	const bool SetupInfoBuffer();
	//Update the structured buffer
	const bool UpdateLightBuffer();
	//Update the constant buffer
	const bool UpdateInfoBuffer();


public:

	Lights();
	~Lights();

	//Sets up Buffers for use in shaders
	bool Initialize();
	
	//Returns true if it's been Initialized
	const bool IsInitialize() const;

	//Render a lights
	void Render(ID3D11DeviceContext* dc);

	//Edit the light in m_lights at index
	void EditLight(light_t L, const int& index = -1);

	//Everytime a new Entity gets the Light Component add it to m_lights
	void Add(entt::registry& reg, entt::entity ent);
};