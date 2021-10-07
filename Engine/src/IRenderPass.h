#pragma once
#include "PipelineManager.h"

//--------------------------------------
// Abstract class.
//--------------------------------------
class IRenderPass
{
private:
	std::string m_name;
	bool m_isEnabled = false;

public:
	virtual ~IRenderPass() = default;
	
	bool IsEnabled() { return m_isEnabled; }
	void SetEnable(bool enable) { m_isEnabled = enable; }
	std::string GetName() { return m_name; }
	void SetName(std::string name) { m_name = name; }

	// Methods to override:
	virtual void Initialize() = 0;
	virtual void PreRender(ID3D11DeviceContext* dc, PipelineManager * pm) = 0;
	virtual void Render(Scene* pScene) = 0;
	virtual void PostRender(ID3D11DeviceContext* dc, PipelineManager* pm) = 0;
};