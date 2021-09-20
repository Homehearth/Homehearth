#pragma once
#include "RenderableObject.h"

class MeshRenderObject : public RenderableObject
{
private:

	ID3D11Buffer* m_constantBuffer = nullptr;
	RMesh* m_mesh = nullptr;
	RTexture* m_texture = nullptr;

public:

	MeshRenderObject(RMesh* p_mesh = nullptr, RTexture* p_texture = nullptr,
		ID3D11Buffer* p_cBuffer = nullptr);
	virtual ~MeshRenderObject() override;

	// Inherited via RenderableObject
	virtual void Render(const objectPass::RenderPass& pass) override;

};