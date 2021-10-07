#pragma once
#include "IRenderPass.h"

class BasePass : public IRenderPass
{
private:
	bool m_isEnabled;
	Camera* m_camera;

public:
	BasePass() = default;
	virtual ~BasePass() = default;
		
	void Initialize() override;
	
	void PreRender(ID3D11DeviceContext* dc, PipelineManager* pm) override;
	
	void Render(Scene* pScene) override;
	
	void PostRender(ID3D11DeviceContext* dc, PipelineManager* pm) override;
	
	void PostRender() override;

	void GetCamera(Camera* camera);
	
};

