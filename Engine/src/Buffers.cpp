#include "EnginePCH.h"
#include "Buffers.h"

namespace Buffers
{
	
    bool ConstantBuffer::create(size_t byteWidth)
    {
        if (this->constantBuffer.Get() != nullptr) {
            this->constantBuffer.Reset();
        }

        // Load constantBufferDesc
        D3D11_BUFFER_DESC constantBufferDesc;
        ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));
    	
        // Assert that the constant buffer remains 16-byte aligned.
        assert(byteWidth % 16 != 0 && "ConstantBuffer size must be 16-bytes aligned.");

        constantBufferDesc.ByteWidth = static_cast<UINT>(byteWidth);
        constantBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
        constantBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = 0;
        constantBufferDesc.MiscFlags = 0;

        HRESULT hr; //= pDevice->CreateBuffer(&constantBufferDesc, nullptr, this->constantBuffer.GetAddressOf());

        return !FAILED(hr);
    }

    ID3D11Buffer* const* ConstantBuffer::GetAddressOf() const
    {
        return this->constantBuffer.GetAddressOf();
    }

    ID3D11Buffer* ConstantBuffer::Get() const
    {
        return this->constantBuffer.Get();
    }







    //--------------------------------------------------------------------------------------
    VertexBuffer::VertexBuffer()
	    : vertexCount(0)
    {
    }
  
    bool VertexBuffer::create(const simple_vertex_t * vertexData, size_t size)
    {
        if (this->vertexBuffer.Get() != nullptr) {
            this->vertexBuffer.Reset();
            this->vertexCount = 0;
        }

        // The Description of the Vertex Buffer.
        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));

        bufferDesc.ByteWidth = static_cast<UINT>(sizeof(simple_vertex_t) * size);
        bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;

        // Model Data to be transferred to GPU Buffer.
        D3D11_SUBRESOURCE_DATA subresData;
        ZeroMemory(&subresData, sizeof(D3D11_SUBRESOURCE_DATA));

        subresData.pSysMem = vertexData;
        subresData.SysMemPitch = 0;
        subresData.SysMemSlicePitch = 0;

        HRESULT hr;// = pDevice->CreateBuffer(&bufferDesc, &subresData, this->vertexBuffer.GetAddressOf());

        this->vertexCount = size;
    	
        return !FAILED(hr);
    }

    size_t VertexBuffer::getVertexCount() const
    {
        return this->vertexCount;
    }
	
    ID3D11Buffer* const* VertexBuffer::GetAddressOf() const
    {
        return this->vertexBuffer.GetAddressOf();
    }

    ID3D11Buffer* VertexBuffer::Get() const
    {
        return this->vertexBuffer.Get();
    }







    //--------------------------------------------------------------------------------------
    IndexBuffer::IndexBuffer()
	    : indexCount(0)
    {
    }

    bool IndexBuffer::create(const size_t * indexData, size_t size)
    {
        if (this->indexBuffer.Get() != nullptr) {
            this->indexBuffer.Reset();
            this->indexCount = 0;
        }

        // Load Index Data
        D3D11_BUFFER_DESC indexBufferDesc;
        ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));

        indexBufferDesc.ByteWidth = static_cast<UINT>(size);
        indexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
        indexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.CPUAccessFlags = 0;
        indexBufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA subresData;
        ZeroMemory(&subresData, sizeof(D3D11_SUBRESOURCE_DATA));

        subresData.pSysMem = indexData;
        subresData.SysMemPitch = 0;
        subresData.SysMemSlicePitch = 0;

        HRESULT hr;// = pDevice->CreateBuffer(&indexBufferDesc, &subresData, this->indexBuffer.GetAddressOf());

        this->indexCount = size;

        return !FAILED(hr);
    }

    ID3D11Buffer* const* IndexBuffer::GetAddressOf() const
    {
        return this->indexBuffer.GetAddressOf();
    }

    ID3D11Buffer* IndexBuffer::Get() const
    {
        return this->indexBuffer.Get();
    }

    size_t IndexBuffer::getIndexCount() const
    {
        return this->indexCount;
    }

} // end of namespace.