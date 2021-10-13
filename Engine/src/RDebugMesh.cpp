#include "EnginePCH.h"
#include "RDebugMesh.h"

bool RDebugMesh::CreateVertexBuffer(const std::vector<debug_vertex_t>& vertices)
{
    D3D11_BUFFER_DESC bufferDesc = {};

    bufferDesc.ByteWidth            = static_cast<UINT>(sizeof(debug_vertex_t) * vertices.size());
    bufferDesc.Usage                = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags            = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags       = 0;
    bufferDesc.MiscFlags            = 0;
    bufferDesc.StructureByteStride  = 0;

    D3D11_SUBRESOURCE_DATA data = {};

    data.pSysMem            = &vertices[0];
    data.SysMemPitch        = 0;
    data.SysMemSlicePitch   = 0;

    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&bufferDesc, &data, m_vertexBuffer.GetAddressOf());
    return !FAILED(hr);
}

RDebugMesh::RDebugMesh()
{
	m_nrOfVertices = 0;
}

RDebugMesh::RDebugMesh(const dx::BoundingOrientedBox& bob)
{
    m_nrOfVertices = 0;
    Create(bob);
}

RDebugMesh::~RDebugMesh()
{
}

void RDebugMesh::Render()
{
	UINT offset = 0;
	UINT stride = sizeof(debug_vertex_t);
	D3D11Core::Get().DeviceContext()->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    D3D11Core::Get().DeviceContext()->Draw(m_nrOfVertices, 0);
}

bool RDebugMesh::Create(const dx::BoundingOrientedBox& bob)
{
    std::vector<debug_vertex_t> vertices;
	
    dx::XMFLOAT3 dxCorners[dx::BoundingOrientedBox::CORNER_COUNT];
    bob.GetCorners(dxCorners);

    debug_vertex_t corners[dx::BoundingOrientedBox::CORNER_COUNT];
    for (UINT i = 0; i < dx::BoundingOrientedBox::CORNER_COUNT; i++)
    {
        corners[i].position = { dxCorners[i].x, dxCorners[i].y, dxCorners[i].z };
    }

    vertices.push_back(corners[1]);
    vertices.push_back(corners[0]);
    vertices.push_back(corners[3]);
    vertices.push_back(corners[2]);

    vertices.push_back(corners[1]);
    vertices.push_back(corners[5]);
    vertices.push_back(corners[6]);
    vertices.push_back(corners[2]);
    
    vertices.push_back(corners[3]);
    vertices.push_back(corners[7]);
    vertices.push_back(corners[6]);
    vertices.push_back(corners[7]);
    
    vertices.push_back(corners[4]);
    vertices.push_back(corners[0]);
    vertices.push_back(corners[4]);
    vertices.push_back(corners[5]);

    m_nrOfVertices = static_cast<UINT>(vertices.size());

    if (!CreateVertexBuffer(vertices))
    {
#ifdef _DEBUG
        LOG_ERROR("Failed to load vertexbuffer for debugmesh...");
#endif // _DEBUG
        vertices.clear();
        return false;
    }

    vertices.clear();
	return true;

    dx::BoundingSphere bs;

}

bool RDebugMesh::Create(const std::string& filename)
{
	return false;
}
