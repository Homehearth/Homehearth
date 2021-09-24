#include "EnginePCH.h"
#include "Shader.h"

namespace Shaders
{

    //---------------------------------------------------------------------------------------------------

    bool IShaders::LoadShaderDataFromFile(const std::string& filename)
    {
        m_filename = filename;

        std::ifstream reader;
        std::string shaderData;
        reader.open(filename + ".cso", std::ios::binary | std::ios::ate);
        if (!reader.is_open())
        {
            LOG_WARNING("failed to open %s.cso", filename.c_str());
            return false;
        }
        reader.seekg(0, std::ios::end);
        shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
        reader.seekg(0, std::ios::beg);
        shaderData.assign((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());
        m_shaderByteCode = shaderData;
        reader.close();

        return true;
    }
	
    std::string IShaders::GetShaderByteCode() const
    {
        return m_shaderByteCode;
    }

    //---------------------------------------------------------------------------------------------------
	
    bool VertexShader::Create(const std::string & filename) 
    {
        HRESULT hr = S_FALSE;
        if(LoadShaderDataFromFile(filename))
        {
			hr = D3D11Core::Get().Device()->CreateVertexShader(GetShaderByteCode().c_str(), GetShaderByteCode().length(), nullptr, m_shader.GetAddressOf());       
        }

        return !FAILED(hr);
    }

    ID3D11VertexShader* const* VertexShader::GetAddressOf() const
    {
        return m_shader.GetAddressOf();
    }

    ID3D11VertexShader* VertexShader::Get() const
    {
        return m_shader.Get();
    }

	
	//---------------------------------------------------------------------------------------------------
	
    bool PixelShader::Create(const std::string& filename)
    {
        HRESULT hr = S_FALSE;
        if (LoadShaderDataFromFile(filename))
        {
            hr = D3D11Core::Get().Device()->CreatePixelShader(GetShaderByteCode().c_str(), GetShaderByteCode().length(), nullptr, m_shader.GetAddressOf());
        }
    	
        return !FAILED(hr);
    }

    ID3D11PixelShader* const* PixelShader::GetAddressOf() const
    {
        return m_shader.GetAddressOf();
    }

    ID3D11PixelShader* PixelShader::Get() const
    {
        return m_shader.Get();
    }

    //---------------------------------------------------------------------------------------------------

    bool ComputeShader::Create(const std::string& filename)
    {
        HRESULT hr = S_FALSE;
        if (LoadShaderDataFromFile(filename))
        {
            hr = D3D11Core::Get().Device()->CreateComputeShader(GetShaderByteCode().c_str(), GetShaderByteCode().length(), nullptr, m_shader.GetAddressOf());
        }
    	
        return !FAILED(hr);
    }

    ID3D11ComputeShader* const* ComputeShader::GetAddressOf() const
    {
        return m_shader.GetAddressOf();
    }

    ID3D11ComputeShader* ComputeShader::Get() const
    {
        return m_shader.Get();
    }

    //---------------------------------------------------------------------------------------------------

    bool GeometryShader::Create(const std::string& filename)
    {
        HRESULT hr = S_FALSE;
        if (LoadShaderDataFromFile(filename))
        {
            hr = D3D11Core::Get().Device()->CreateGeometryShader(GetShaderByteCode().c_str(), GetShaderByteCode().length(), nullptr, m_shader.GetAddressOf());
        }
    	
        return !FAILED(hr);
    }

    ID3D11GeometryShader* const* GeometryShader::GetAddressOf() const
    {
        return m_shader.GetAddressOf();
    }

    ID3D11GeometryShader* GeometryShader::Get() const
    {
        return m_shader.Get();
    }

    //---------------------------------------------------------------------------------------------------

    bool HullShader::Create(const std::string& filename)
    {
        HRESULT hr = S_FALSE;
        if (LoadShaderDataFromFile(filename))
        {
            hr = D3D11Core::Get().Device()->CreateHullShader(GetShaderByteCode().c_str(), GetShaderByteCode().length(), nullptr, m_shader.GetAddressOf());
        }
    	
        return !FAILED(hr);
    }

    ID3D11HullShader* const* HullShader::GetAddressOf() const
    {
        return m_shader.GetAddressOf();
    }

    ID3D11HullShader* HullShader::Get() const
    {
        return m_shader.Get();
    }

    //---------------------------------------------------------------------------------------------------

    bool DomainShader::Create(const std::string& filename)
    {
        HRESULT hr = S_FALSE;
        if (LoadShaderDataFromFile(filename))
        {
            hr = D3D11Core::Get().Device()->CreateDomainShader(GetShaderByteCode().c_str(), GetShaderByteCode().length(), nullptr, m_shader.GetAddressOf());
        }
    	
        return !FAILED(hr);
    }

    ID3D11DomainShader* const* DomainShader::GetAddressOf() const
    {
        return m_shader.GetAddressOf();
    }

    ID3D11DomainShader* DomainShader::Get() const
    {
        return m_shader.Get();
    }

    //---------------------------------------------------------------------------------------------------
	
}   // end of namespace.