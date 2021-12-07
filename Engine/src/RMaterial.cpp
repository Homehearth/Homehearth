#include "EnginePCH.h"
#include "RMaterial.h"

RMaterial::RMaterial()
{
    m_isTransparent = false;
}

RMaterial::~RMaterial()
{
}

const std::string RMaterial::GetFilename(const std::string& path) const
{
    //Split up the string path to only the textures filename
    size_t index = path.find_last_of("/\\");
    return path.substr(index + 1);
}

bool RMaterial::LoadTexture(const ETextureType& type, const std::string& filename)
{
    //Check if the resource exist. Don't create if it does not exist
    std::shared_ptr<RTexture> texture = ResourceManager::Get().GetResource<RTexture>(filename, false);

    //Resource did not exist, we shall create it with the right format
    if (!texture)
    {
        //Has only one channel
        if (type == ETextureType::metalness         ||
            type == ETextureType::roughness         ||
            type == ETextureType::ambientOcclusion  ||
            type == ETextureType::displacement      ||
            type == ETextureType::opacitymask)
        {
            texture = std::make_shared<RTexture>(ETextureChannelType::oneChannel);
        }
        else
        {
            texture = std::make_shared<RTexture>();
        }

        if (!texture->Create(filename))
        {
            return false;
        }
        ResourceManager::Get().AddResource(filename, texture);
    }
    
    m_textures[(uint8_t)type] = texture;
    return true;
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

    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&desc, &data, m_hasTextureCB.GetAddressOf());
    return !FAILED(hr);
}

void RMaterial::BindMaterial(ID3D11DeviceContext* context)
{
    /*
        Bind the constant buffers
    */
    context->PSSetConstantBuffers(CB_PROPERTIES_SLOT,  1, m_hasTextureCB.GetAddressOf());

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
    context->PSSetShaderResources(T2D_STARTSLOT, nrOfTextures, allSRV);
}

void RMaterial::UnBindMaterial(ID3D11DeviceContext* context)
{
    //Unbind the constantbuffers
    ID3D11Buffer* nullBuffer = nullptr;
    context->PSSetConstantBuffers(CB_PROPERTIES_SLOT,  1, &nullBuffer);

    //Unbind all the textures
    const UINT nrOfTextures = UINT(ETextureType::length);
    ID3D11ShaderResourceView* nullSRV[nrOfTextures] = { nullptr };
    context->PSSetShaderResources(T2D_STARTSLOT, nrOfTextures, nullSRV);
}

bool RMaterial::HasTexture(const ETextureType& type) const
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

const std::shared_ptr<RTexture> RMaterial::GetTexture(const ETextureType& type) const
{
    std::shared_ptr<RTexture> texture;

    if (HasTexture(type))
    {
        texture = m_textures[(UINT)type];
    }
    return texture;
}

bool RMaterial::IsTransparent() const
{
    return m_isTransparent;
}

bool RMaterial::Create(aiMaterial* aiMat)
{
    float transparency = 1.0f;
    if (AI_SUCCESS == aiMat->Get(AI_MATKEY_OPACITY, transparency))
        m_properties.transparency = transparency;

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
        {ETextureType::displacement,        aiTextureType::aiTextureType_DISPLACEMENT},
        {ETextureType::opacitymask,         aiTextureType::aiTextureType_OPACITY}
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
            LoadTexture(type.first, filename);
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
        m_properties.hasAoMap = true;
    if (m_textures[(uint8_t)ETextureType::displacement])
        m_properties.hasDisplace = true;
    if (m_textures[(uint8_t)ETextureType::opacitymask])
        m_properties.hasOpacity = true;

    if (!CreateConstBuf(m_properties))
    {
#ifdef _DEBUG
        LOG_WARNING("Failed to create constantbuffer for 'hasTextures'");
#endif 
        return false;
    }

    /*
        Check if the material should be counted as transparent
    */
    if (m_textures[(uint8_t)ETextureType::albedo])
    {
        if (m_textures[(uint8_t)ETextureType::albedo]->IsTransparent())
            m_isTransparent = true;
    }
    else if (m_properties.transparency < 1.0f)
        m_isTransparent = true;

    return true;
}

bool RMaterial::CreateFromMTL(std::string& text)
{
    /*
        Split the text to blocks for every line
    */
    std::vector<std::string> allLines;
    while (!text.empty())
    {
        size_t nextLine = text.find('\n');
        std::string line = text.substr(0, nextLine);
        text.erase(0, nextLine + 1);
        allLines.push_back(line);
    }
    
    /*
        Finally loading in the data
    */
    for (size_t i = 0; i < allLines.size(); i++)
    {
        std::stringstream ss(allLines[i]);
        std::string prefix;
        ss >> prefix;

        //Transparency/ opacity
        if (prefix == "d")
        {
            ss >> m_properties.transparency;
        }
        //Albedo map
        else if (prefix == "map_Kd")
        {
            std::string filepath;
            if (ss >> filepath)
            {
                std::string filename = GetFilename(filepath);
                if (LoadTexture(ETextureType::albedo, filename))
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
                if (LoadTexture(ETextureType::normal, filename))
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
                if (LoadTexture(ETextureType::metalness, filename))
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
                if (LoadTexture(ETextureType::roughness, filename))
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
                if (LoadTexture(ETextureType::ambientOcclusion, filename))
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
                if (LoadTexture(ETextureType::displacement, filename))
                    m_properties.hasDisplace = true;
            }
        }
        //Opacity map
        else if (prefix == "map_d")
        {
            std::string filepath;
            if (ss >> filepath)
            {
                std::string filename = GetFilename(filepath);
                if (LoadTexture(ETextureType::opacitymask, filename))
                    m_properties.hasOpacity = true;
            }
        }

    }

    if (!CreateConstBuf(m_properties))
    {
#ifdef _DEBUG
        LOG_WARNING("Failed to create constantbuffer for 'hasTextures'");
#endif 
        return false;
    }

    /*
        Check if the material should be counted as transparent
    */
    if (m_textures[(uint8_t)ETextureType::albedo]->IsTransparent())
        m_isTransparent = true;
    else if (m_properties.transparency < 1.0f)
        m_isTransparent = true;
    
    return true;
}

bool RMaterial::Create(const std::string& filename)
{
    //Don't know what to do here...
    return false; 
}
