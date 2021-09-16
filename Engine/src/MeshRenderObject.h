#pragma once
#include "RenderableObject.h"

class MeshRenderObject : public RenderableObject
{
private:

	ID3D11Buffer* m_constantBuffer;
	RMesh* m_mesh;
	RTexture* m_texture;

public:

	MeshRenderObject(RMesh* p_mesh = nullptr, RTexture* p_texture = nullptr,
		ID3D11Buffer* p_cBuffer = nullptr);

	// Inherited via RenderableObject
	virtual void Render() override;

};