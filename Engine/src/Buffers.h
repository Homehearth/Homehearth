#pragma once
#include "MeshStructure.h"

namespace Buffers
{

	class ConstantBuffer
	{
	private:
		ComPtr<ID3D11Buffer> constantBuffer;
	
	public:
		ConstantBuffer() = default;
		ConstantBuffer(const ConstantBuffer& other) = delete;
		ConstantBuffer(ConstantBuffer&& other) = delete;
		ConstantBuffer& operator=(const ConstantBuffer& other) = delete;
		ConstantBuffer& operator=(ConstantBuffer&& other) = delete;
		virtual ~ConstantBuffer() = default;

		bool create(size_t byteWidth);

		ID3D11Buffer* const* GetAddressOf() const;
		ID3D11Buffer* Get() const;
	};

	class VertexBuffer
	{
	private:
		ComPtr<ID3D11Buffer> vertexBuffer;
		size_t vertexCount;
	
	public:
		VertexBuffer();
		VertexBuffer(const VertexBuffer& other) = delete;
		VertexBuffer(VertexBuffer&& other) = delete;
		VertexBuffer& operator=(const VertexBuffer& other) = delete;
		VertexBuffer& operator=(VertexBuffer&& other) = delete;
		virtual ~VertexBuffer() = default;

		bool create(const simple_vertex_t* vertexData, size_t size);

		size_t getVertexCount() const;
		ID3D11Buffer* const* GetAddressOf() const;
		ID3D11Buffer* Get() const;
	};

	class IndexBuffer
	{
	private:
		ComPtr<ID3D11Buffer> indexBuffer;
		size_t indexCount;
	
	public:
		IndexBuffer();
		IndexBuffer(const IndexBuffer& other) = delete;
		IndexBuffer(IndexBuffer&& other) = delete;
		IndexBuffer& operator=(const IndexBuffer& other) = delete;
		IndexBuffer& operator=(IndexBuffer&& other) = delete;
		virtual ~IndexBuffer() = default;

		bool create(const size_t * indexData, size_t size);

		size_t getIndexCount() const;
		ID3D11Buffer* const* GetAddressOf() const;
		ID3D11Buffer* Get() const;
		
	};

} // End of namespace.