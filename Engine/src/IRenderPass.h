#pragma once
#include "PipelineManager.h"

//--------------------------------------
// Abstract class: header file only.
//--------------------------------------
class IRenderPass
{
private:
	std::string m_name = "Unknown";
	bool m_isEnabled = false;
	Camera* m_camera = nullptr;
	Scene* m_scene = nullptr;
	ID3D11DeviceContext* m_deviceContext = nullptr;
	PipelineManager* m_pipelineManager = nullptr;

public:
	virtual ~IRenderPass() = default;

	// Methods to override:
	virtual void Initialize() = 0;
	virtual void PreRender() = 0;
	virtual void Render() = 0;
	virtual void PostRender() = 0;

	
	// Setters.
	void SetEnable(bool enable) { m_isEnabled = enable; }
	void SetName(const std::string &name) { m_name = name; }
	void SetCamera(Camera* pCamera) { m_camera = pCamera; }
	void SetScene(Scene* pScene) { m_scene = pScene; }
	void SetContext(ID3D11DeviceContext* pDeviceContext) { m_deviceContext = pDeviceContext; }
	void SetPipelineManager(PipelineManager* pPipelineManager) { m_pipelineManager = pPipelineManager; }

	// Getters.
	bool IsEnabled() const { return m_isEnabled; }
	std::string GetName() const { return m_name; }
	Camera* GetCamera() const { return m_camera; }
	Scene* GetScene() const { return m_scene; }
	ID3D11DeviceContext* GetContext() const { return m_deviceContext; }
	PipelineManager* GetPipelineManager() const { return m_pipelineManager; }

	// May be used to clear the entire pipeline.
	void ClearPipelineSettings() const
	{
		if (m_deviceContext != nullptr)
		{
			ID3D11ShaderResourceView* nullSRV[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
													nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

			ID3D11RenderTargetView* nullRTV[] = {	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
													nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

			ID3D11Buffer* nullBuffer[] = {	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
											nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, };

			ID3D11SamplerState* nullSampler[] = {	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
													nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

			ID3D11DepthStencilView* nullDSV = nullptr;
			
			// Shaders.
			m_deviceContext->VSSetShader(nullptr,nullptr, 0);
			m_deviceContext->PSSetShader(nullptr,nullptr, 0);
			m_deviceContext->CSSetShader(nullptr,nullptr, 0);
			m_deviceContext->HSSetShader(nullptr,nullptr, 0);
			m_deviceContext->GSSetShader(nullptr,nullptr, 0);
			
			// Constant buffers.
			m_deviceContext->VSSetConstantBuffers(0, ARRAYSIZE(nullBuffer), nullBuffer);
			m_deviceContext->PSSetConstantBuffers(0, ARRAYSIZE(nullBuffer), nullBuffer);
			m_deviceContext->CSSetConstantBuffers(0, ARRAYSIZE(nullBuffer), nullBuffer);

			// Resources.
			m_deviceContext->VSSetShaderResources(0, ARRAYSIZE(nullSRV), nullSRV);
			m_deviceContext->PSSetShaderResources(0, ARRAYSIZE(nullSRV), nullSRV);
			m_deviceContext->CSSetShaderResources(0, ARRAYSIZE(nullSRV), nullSRV);

			// Samplers.
			m_deviceContext->VSSetSamplers(0, ARRAYSIZE(nullSampler), nullSampler);
			m_deviceContext->PSSetSamplers(0, ARRAYSIZE(nullSampler), nullSampler);
			m_deviceContext->CSSetSamplers(0, ARRAYSIZE(nullSampler), nullSampler);

			// Render targets.
			m_deviceContext->OMSetRenderTargets(8, nullRTV, nullDSV);

			// States.
			ID3D11DepthStencilState* nullDepthStencilState = nullptr;
			ID3D11RasterizerState* nullRasterState = nullptr;
			m_deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
			m_deviceContext->OMSetDepthStencilState(nullDepthStencilState, 0x00);
			m_deviceContext->RSSetState(nullRasterState);
		}
	}	
};