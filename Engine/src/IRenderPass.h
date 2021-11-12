#pragma once
#include "PipelineManager.h"
#include "Lights.h"

// Only to be used inside a RenderPass (!).
#define DC pDeviceContext
#define PM GetPipelineManager()

//--------------------------------------
// Abstract class. Header file only.
//--------------------------------------
class IRenderPass
{
private:
	ID3D11DeviceContext* m_contextDevice = nullptr;
	PipelineManager* m_pipelineManager = nullptr;
	bool m_isEnable = false;

protected:
	Lights* m_lights;

public:
	virtual ~IRenderPass() = default;
	
	void Initialize(ID3D11DeviceContext* pContextDevice, PipelineManager* pPipelineManager)
	{
		m_contextDevice = pContextDevice;
		m_pipelineManager = pPipelineManager;
	}

	bool IsEnabled() const { return m_isEnable; }
	void SetEnable(bool enable) { m_isEnable = enable; } 
	ID3D11DeviceContext* GetDeviceContext() const { return m_contextDevice; }
	PipelineManager* GetPipelineManager() const { return m_pipelineManager; }
	
	// Methods to override.
	virtual void PreRender(Camera* pCam = nullptr, ID3D11DeviceContext * pDeviceContext = D3D11Core::Get().DeviceContext()) = 0;
	virtual void Render(Scene* pScene) = 0;
	virtual void PostRender(ID3D11DeviceContext* pDeviceContext = D3D11Core::Get().DeviceContext()) = 0;
	virtual void SetLights(Lights* light);
};