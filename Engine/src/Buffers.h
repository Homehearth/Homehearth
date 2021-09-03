#pragma once

struct simple_vertex_t {};

namespace Buffers
{

	class ConstantBuffer
	{
	public:
		ConstantBuffer();
		ConstantBuffer(const ConstantBuffer& other) = delete;
		ConstantBuffer(ConstantBuffer&& other) = delete;
		ConstantBuffer& operator=(const ConstantBuffer& other) = delete;
		ConstantBuffer& operator=(ConstantBuffer&& other) = delete;
		virtual ~ConstantBuffer() = default;

		bool createConstantBuffer(size_t byteWidth);

		ID3D11Buffer* const* GetAddressOf() const;
		ID3D11Buffer* Get() const;
	private:
		ComPtr<ID3D11Buffer> constantBuffer;
	};

	class VertexBuffer
	{
	public:
		VertexBuffer();
		VertexBuffer(const VertexBuffer& other) = delete;
		VertexBuffer(VertexBuffer&& other) = delete;
		VertexBuffer& operator=(const VertexBuffer& other) = delete;
		VertexBuffer& operator=(VertexBuffer&& other) = delete;
		virtual ~VertexBuffer() = default;

		bool createVertexBuffer(const simple_vertex_t* vertexData, size_t size);

		size_t getVertexCount() const;
		size_t getVertexStride() const;
		ID3D11Buffer* const* GetAddressOf() const;
		ID3D11Buffer* Get() const;
	private:
		ComPtr<ID3D11Buffer> vertexBuffer;
		size_t vertexCount;
	};

	class IndexBuffer
	{
	public:
		IndexBuffer();
		IndexBuffer(const IndexBuffer& other) = delete;
		IndexBuffer(IndexBuffer&& other) = delete;
		IndexBuffer& operator=(const IndexBuffer& other) = delete;
		IndexBuffer& operator=(IndexBuffer&& other) = delete;
		virtual ~IndexBuffer() = default;

		bool createIndexBuffer(const size_t * indexData, size_t size);

		size_t getIndexCount() const;
		ID3D11Buffer* const* GetAddressOf() const;
		ID3D11Buffer* Get() const;
	private:
		ComPtr<ID3D11Buffer> indexBuffer;
		size_t indexCount;
	};

} // end of namespace.