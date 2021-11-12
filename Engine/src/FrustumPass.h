#pragma once
class FrustumPass : public IRenderPass
{
private:
	const int TILE_SIZE = 16; // Must have the same value in shaders.
	unsigned int m_numFrustums;

	dispatch_params_t m_dispatchParams;

	bool ComputeGridFrustums();
	bool CreateGridFrustumSWB(size_t numElements, size_t elementSize, const void* bufferData = nullptr) const;

public:

	// Recompute frustums upon window resize.
	bool UpdateFrustums();

	void PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext) override;

	void Render(Scene* pScene) override;

	void PostRender(ID3D11DeviceContext* pDeviceContext) override;
};

