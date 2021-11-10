#pragma once

class LightCullingPass : IRenderPass
{
private:
	const int TILE_SIZE = 16; // Must have the same value in shaders.

	static bool m_hasChanged;

	unsigned int m_numFrustums;
	dispatch_params_t m_dispatchParams;

	DirectX::ConstantBuffer<dispatch_params_t> dispatchParamsBuffer;
	DirectX::ConstantBuffer<screen_view_params_t> screenToViewParams;

	// Created once, and updated upon OnWindowResize. 
	void ComputeGridFrustums();
	bool CreateGridFrustumSWB(size_t numElements, size_t elementSize, const void* bufferData = nullptr) const;
public:
	LightCullingPass() = default;
	virtual ~LightCullingPass() = default;

	void PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;

	void Render(Scene* pScene) override;

	void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) override;
};


