#include "EnginePCH.h"
#include "MeshRenderObject.h"

MeshRenderObject::MeshRenderObject(RMesh* p_mesh, RTexture* p_texture, ID3D11Buffer* p_cBuffer)
{
	/*
		Assign each pointer to this object, Also add a reference so that
		they dont get deleted midway rendering.
	*/
	if (p_mesh)
		[&] {p_mesh->AddRef(); m_mesh = p_mesh; };
	if (p_texture)
		[&] {p_texture->AddRef(); m_texture = p_texture; };
	if (p_cBuffer)
		[&] {p_cBuffer->AddRef(); m_constantBuffer = p_cBuffer; };
}

MeshRenderObject::~MeshRenderObject()
{
	using namespace resource;
	SafeRelease(m_mesh);
	SafeRelease(m_texture);
	if (m_constantBuffer)
		m_constantBuffer->Release();
}

void MeshRenderObject::Render()
{
#define CONTEXT D3D11Core::Get().DeviceContext()
	if (m_texture)
	{
		CONTEXT->PSSetShaderResources(0, 1, &m_texture->GetShaderView());
	}

	if (m_mesh)
		m_mesh->Render();
}
