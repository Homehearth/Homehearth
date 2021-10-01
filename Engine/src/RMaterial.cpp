#include "EnginePCH.h"
#include "RMaterial.h"

/*
    [Tweak]: Bind constantbuffers to specific locations/numbers
    search in file for "[Tweak]" if needed
*/

RMaterial::RMaterial()
{
}

RMaterial::~RMaterial()
{
}

std::string RMaterial::GetFilename(const std::string& path)
{
    //Split up the string path to only the textures filename
    size_t index = path.find_last_of("/\\");
    return path.substr(index + 1);
}

bool RMaterial::CreateConstBuf(const matConstants_t& mat)
{
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth           = sizeof(matConstants_t);
    desc.Usage               = D3D11_USAGE_DYNAMIC;
    desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags           = 0;
    desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem            = &mat;
    data.SysMemPitch        = 0;
    data.SysMemSlicePitch   = 0;

    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&desc, &data, &m_matConstCB);
    return !FAILED(hr);
}

bool RMaterial::CreateConstBuf(const properties_t& mat)
{
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth           = sizeof(properties_t);
    desc.Usage               = D3D11_USAGE_DYNAMIC;
    desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags           = 0;
    desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem            = &mat;
    data.SysMemPitch        = 0;
    data.SysMemSlicePitch   = 0;

    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&desc, &data, &m_hasTextureCB);
    return !FAILED(hr);
}

void RMaterial::BindMaterial()
{
    /*
        Bind the constant buffers
        [Tweak]
    */
    D3D11Core::Get().DeviceContext()->PSSetConstantBuffers(0, 1, &m_matConstCB);
    D3D11Core::Get().DeviceContext()->PSSetConstantBuffers(1, 1, &m_hasTextureCB);

    /*
        Upload all textures
    */
    //Get every shader resource view
    const UINT nrOfTextures = UINT(ETextureType::length);
    ID3D11ShaderResourceView* allSRV[nrOfTextures] = { nullptr };
    for (UINT i = 0; i < nrOfTextures; i++)
    {
        //Texture has to exist
        if (m_textures[i])
            allSRV[i] = m_textures[i]->GetShaderView();
    }
    //Bind all the textures to the GPU's pixelshader
    D3D11Core::Get().DeviceContext()->PSSetShaderResources(0, nrOfTextures, allSRV);
}

void RMaterial::UnBindMaterial()
{
    //Unbind the constantbuffers
    ID3D11Buffer* nullBuffer = nullptr;
    D3D11Core::Get().DeviceContext()->PSSetConstantBuffers(0, 1, &nullBuffer);
    D3D11Core::Get().DeviceContext()->PSSetConstantBuffers(1, 1, &nullBuffer);

    //Unbind all the textures
    const UINT nrOfTextures = UINT(ETextureType::length);
    ID3D11ShaderResourceView* nullSRV[nrOfTextures] = { nullptr };
    D3D11Core::Get().DeviceContext()->PSSetShaderResources(0, nrOfTextures, nullSRV);
}

bool RMaterial::HasTexture(const ETextureType& type)
{
    bool foundTexture = false;

    //Ignore length type
    if (type != ETextureType::length)
    {
        if (m_textures[(UINT)type])
            foundTexture = true;
    }
    return foundTexture;
}

bool RMaterial::Create(aiMaterial* aiMat)
{
    /*
        Load in material constants
    */
    matConstants_t matConst;
    aiColor3D ambient = { 0.f, 0.f, 0.f };
    if (AI_SUCCESS == aiMat->Get(AI_MATKEY_COLOR_AMBIENT, ambient))
        matConst.ambient = { ambient.r, ambient.g, ambient.b };
    
    aiColor3D diffuse = { 0.f, 0.f, 0.f };
    if (AI_SUCCESS == aiMat->Get(AI_MATKEY_COLOR_DIFFUSE,  diffuse))
        matConst.diffuse = { diffuse.r, diffuse.g, diffuse.b };
    
    aiColor3D specular = { 0.f, 0.f, 0.f };
    if (AI_SUCCESS == aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specular))
        matConst.specular = { specular.r, specular.g, specular.b };

    float shiniess = 0.0f;
    if (AI_SUCCESS == aiMat->Get(AI_MATKEY_SHININESS, shiniess))
        matConst.shiniess = shiniess;
    
    
    if (!CreateConstBuf(matConst))
    {
#ifdef _DEBUG
        LOG_WARNING("Failed to create constantbuffer for material constants");
#endif 
        return false;
    }

    /*
        Load in textures
    */
    //Syncing the enums between assimp and this structure
    std::unordered_map<ETextureType, aiTextureType> textureTypeMap =
    {
        {ETextureType::albedo,              aiTextureType::aiTextureType_DIFFUSE},
        {ETextureType::normal,              aiTextureType::aiTextureType_NORMALS},
        {ETextureType::metalness,           aiTextureType::aiTextureType_SHININESS},
        {ETextureType::roughness,           aiTextureType::aiTextureType_SPECULAR},
        {ETextureType::ambientOcclusion,    aiTextureType::aiTextureType_AMBIENT},
        {ETextureType::displacement,        aiTextureType::aiTextureType_DISPLACEMENT}
    };

    //For every texturetype: add the texture to the map
    for (auto& type : textureTypeMap)
    {
        aiString path;
        //Get the filepath from Assimp
        if (AI_SUCCESS == aiMat->GetTexture(type.second, 0, &path))
        {
            //Strip down path to only filename
            std::string filename = GetFilename(path.C_Str());
            //Add the resource and create it
            m_textures[(uint8_t)type.first] = ResourceManager::Get().GetResource<RTexture>(filename);
        }
    }
    textureTypeMap.clear();

    //Create constbuffer with what textures that exist
    if (m_textures[(uint8_t)ETextureType::albedo])
        m_properties.hasAlbedo = true;
    if (m_textures[(uint8_t)ETextureType::normal])
        m_properties.hasNormal = true;
    if (m_textures[(uint8_t)ETextureType::metalness])
        m_properties.hasMetalness = true;
    if (m_textures[(uint8_t)ETextureType::roughness])
        m_properties.hasRoughness = true;
    if (m_textures[(uint8_t)ETextureType::ambientOcclusion])
        m_properties.hasAlbedo = true;
    if (m_textures[(uint8_t)ETextureType::displacement])
        m_properties.hasDisplace = true;

    if (!CreateConstBuf(m_properties))
    {
#ifdef _DEBUG
        LOG_WARNING("Failed to create constantbuffer for 'hasTextures'");
#endif 
        return false;
    }

    return true;
}

bool RMaterial::Create(const std::string& filename)
{
    //Load a mtl file
    std::string filepath = MATERIALPATH + filename;
    std::ifstream readfile(filepath);
    
    //Failed to open file
    if (!readfile.is_open())
    {
        return false;
    }

	std::string line;
	matConstants_t matConst;

	while (std::getline(readfile, line))
	{
		std::stringstream ss(line);
		std::string prefix;
		ss >> prefix;

		/*
			Basic material information
		*/
		//Ambient
		if (prefix == "Ka")
		{
			ss >> matConst.ambient.x >> matConst.ambient.y >> matConst.ambient.z;
		}
		//Diffuse
		else if (prefix == "Kd")
		{
			ss >> matConst.diffuse.x;
			ss >> matConst.diffuse.y;
			ss >> matConst.diffuse.z;
		}
		//Specular
		else if (prefix == "Ks")
		{
			ss >> matConst.specular.x;
			ss >> matConst.specular.y;
			ss >> matConst.specular.z;
		}
		//Shiniess
		else if (prefix == "Ns")
		{
			ss >> matConst.shiniess;
		}
		//Transparency/ opacity
		else if (prefix == "Tr")
		{
			ss >> matConst.opacity;
		}

		/*
			Textures
		*/
		//Albedo map
		else if (prefix == "map_Kd")
		{
			std::string filepath;
			if (ss >> filepath)
			{
				std::string filename = GetFilename(filepath);
				m_textures[(uint8_t)ETextureType::albedo] = ResourceManager::Get().GetResource<RTexture>(filename);
                m_properties.hasAlbedo = true;
			}
		}
		//Normalmap
		else if (prefix == "map_Kn")
		{
			std::string filepath;
			if (ss >> filepath)
			{
				std::string filename = GetFilename(filepath);
				m_textures[(uint8_t)ETextureType::normal] = ResourceManager::Get().GetResource<RTexture>(filename);
                m_properties.hasNormal = true;
			}
		}
		//Metallic
		else if (prefix == "map_ns")
		{
			std::string filepath;
			if (ss >> filepath)
			{
				std::string filename = GetFilename(filepath);
				m_textures[(uint8_t)ETextureType::metalness] = ResourceManager::Get().GetResource<RTexture>(filename);
                m_properties.hasMetalness = true;
			}
		}
		//Roughness
		else if (prefix == "map_Ks")
		{
			std::string filepath;
			if (ss >> filepath)
			{
				std::string filename = GetFilename(filepath);
				m_textures[(uint8_t)ETextureType::roughness] = ResourceManager::Get().GetResource<RTexture>(filename);
                m_properties.hasRoughness = true;
			}
		}
		//Ambient occulution map
		else if (prefix == "map_Ka")
		{
			std::string filepath;
			if (ss >> filepath)
			{
				std::string filename = GetFilename(filepath);
				m_textures[(uint8_t)ETextureType::ambientOcclusion] = ResourceManager::Get().GetResource<RTexture>(filename);
                m_properties.hasAoMap = true;
			}
		}
        //Displacement map
        else if (prefix == "map_disp")
        {
            std::string filepath;
            if (ss >> filepath)
            {
                std::string filename = GetFilename(filepath);
                m_textures[(uint8_t)ETextureType::displacement] = ResourceManager::Get().GetResource<RTexture>(filename);
                m_properties.hasDisplace = true;
            }
        }
	}
	readfile.close();

	if (!CreateConstBuf(matConst))
	{
#ifdef _DEBUG
		LOG_WARNING("Failed to create constantbuffer for material constants");
#endif 
		return false;
	}

	if (!CreateConstBuf(m_properties))
	{
#ifdef _DEBUG
		LOG_WARNING("Failed to create constantbuffer for 'hasTextures'");
#endif 
		return false;
	}

    return true; 
}
