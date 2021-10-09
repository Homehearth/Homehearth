#pragma once
#include "PipelineManager.h"

// Only to be used inside a RenderPass (!).
#define CAMERA GetCamera()
#define DC GetDeviceContext()
#define PM GetPipelineManager()

//--------------------------------------
// Abstract class. Header file only.
//--------------------------------------
class IRenderPass
{
private:
	Camera* m_camera = nullptr;
	ID3D11DeviceContext* m_contextDevice = nullptr;
	PipelineManager* m_pipelineManager = nullptr;
	bool m_isEnable = false;

public:
	virtual ~IRenderPass() = default;
	
	virtual void Initialize(Camera* pCamera, ID3D11DeviceContext* pContextDevice, PipelineManager* pPipelineManager)
	{
		m_camera = pCamera;
		m_contextDevice = pContextDevice;
		m_pipelineManager = pPipelineManager;
	}	
	bool IsEnabled() const { return m_isEnable; }
	void SetEnable(bool enable) { m_isEnable = enable; } 
	Camera* GetCamera() const { return m_camera; }
	ID3D11DeviceContext* GetDeviceContext() const { return m_contextDevice; }
	PipelineManager* GetPipelineManager() const { return m_pipelineManager; }

	
	// Methods to override.
	virtual void PreRender() = 0;
	virtual void Render(Scene* pScene) = 0;
	virtual void PostRender() = 0;
};