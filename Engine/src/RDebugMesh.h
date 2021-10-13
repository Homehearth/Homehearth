#pragma once
#include "CommonStructures.h"

class RDebugMesh : public resource::GResource
{
private:
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	UINT				 m_nrOfVertices;

private:
	bool CreateVertexBuffer(const std::vector<debug_vertex_t>& vertices);

public:
	RDebugMesh();
	~RDebugMesh();

	//Render with vertexbuffer
	void Render();

	//Create the vertices needed for the box to render
	bool Create(const dx::BoundingOrientedBox& bob);

	// Does nothing... Inherited via GResource
	virtual bool Create(const std::string& filename) override;

};