#pragma once
#include "EnginePCH.h"

/*
 * These classes require the D3D11Core to be initialized.
 */
namespace Shaders
{
	
	// Shader Interface.
    class IShaders
    {
    private:
        std::string m_shaderByteCode;
        std::string m_filename;
    protected:
        bool LoadShaderDataFromFile(const std::string& filename);
    public:
        std::string GetShaderByteCode() const;
    };


	
    class VertexShader : public IShaders, public resource::GResource
    {
    private:
        ComPtr<ID3D11VertexShader> m_shader;
    public:
        VertexShader() = default;
        VertexShader(const VertexShader& other) = default;
        VertexShader(VertexShader&& other) = default;
        VertexShader& operator=(const VertexShader& other) = default;
        VertexShader& operator=(VertexShader&& other) = default;
        virtual ~VertexShader() = default;
    	
        bool Create(const std::string& filename) override;

		ID3D11VertexShader* const* GetAddressOf() const;
        ID3D11VertexShader* Get() const;
    };

    class PixelShader : public IShaders, public resource::GResource
    {
    private:
        ComPtr<ID3D11PixelShader> m_shader;
    public:
        PixelShader() = default;
        PixelShader(const PixelShader& other) = default;
        PixelShader(PixelShader&& other) = default;
        PixelShader& operator=(const PixelShader& other) = default;
        PixelShader& operator=(PixelShader&& other) = default;
        virtual ~PixelShader() = default;
    	
        bool Create(const std::string& filename) override;
    	
        ID3D11PixelShader* const* GetAddressOf() const;
        ID3D11PixelShader* Get() const;
    };

    class ComputeShader : public IShaders, public resource::GResource
    {
    private:
        ComPtr<ID3D11ComputeShader> m_shader;
    public:
        ComputeShader() = default;
        ComputeShader(const ComputeShader& other) = delete;
        ComputeShader(ComputeShader&& other) = default;
        ComputeShader& operator=(const ComputeShader& other) = delete;
        ComputeShader& operator=(ComputeShader&& other) = default;
        virtual ~ComputeShader() = default;

        bool Create(const std::string& filename) override;

        ID3D11ComputeShader* const* GetAddressOf() const;
        ID3D11ComputeShader* Get() const;
    };

    class GeometryShader : public IShaders, public resource::GResource
    {
    private:
        ComPtr<ID3D11GeometryShader> m_shader;
    public:
        GeometryShader() = default;
        GeometryShader(const GeometryShader& other) = delete;
        GeometryShader(GeometryShader&& other) = default;
        GeometryShader& operator=(const GeometryShader& other) = delete;
        GeometryShader& operator=(GeometryShader&& other) = default;
        virtual ~GeometryShader() = default;

        bool Create(const std::string& filename) override;

        ID3D11GeometryShader* const* GetAddressOf() const;
        ID3D11GeometryShader* Get() const;
    };

    class HullShader : public IShaders, public resource::GResource
    {
    private:
        ComPtr<ID3D11HullShader> m_shader;
    public:
        HullShader() = default;
        HullShader(const HullShader& other) = delete;
        HullShader(HullShader&& other) = default;
        HullShader& operator=(const HullShader& other) = delete;
        HullShader& operator=(HullShader&& other) = default;
        virtual ~HullShader() = default;

        bool Create(const std::string& filename) override;

        ID3D11HullShader* const* GetAddressOf() const;
        ID3D11HullShader* Get() const;
    };

    class DomainShader : public IShaders, public resource::GResource
    {
    private:
        ComPtr<ID3D11DomainShader> m_shader;
    public:
        DomainShader() = default;
        DomainShader(const DomainShader& other) = delete;
        DomainShader(DomainShader&& other) = default;
        DomainShader& operator=(const DomainShader& other) = delete;
        DomainShader& operator=(DomainShader&& other) = default;
        virtual ~DomainShader() = default;

        bool Create(const std::string& filename) override;

        ID3D11DomainShader* const* GetAddressOf() const;
        ID3D11DomainShader* Get() const;
    };
	
} // end of namespace.

