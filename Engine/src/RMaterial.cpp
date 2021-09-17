#include "EnginePCH.h"
#include "RMaterial.h"

RMaterial::RMaterial()
{
    m_name = "";
    m_ambient  = {};
    m_diffuse  = {};
    m_specular = {};
    m_shiniess = 0.0f;
    m_opacity = 1.0f;

    for (UINT i = 0; i < (UINT)ETextureType::length; i++)
        m_textures[i] = nullptr;
}

RMaterial::~RMaterial()
{
}

void RMaterial::UploadToGPU()
{
    //Upload the material to some kind of constantbuffer?
}

void RMaterial::LoadMaterial(aiMaterial* aiMat)
{
    /*
        Load in material numbers
    */
    aiMat->Get(AI_MATKEY_COLOR_AMBIENT, m_ambient);
    aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, m_diffuse);
    aiMat->Get(AI_MATKEY_COLOR_SPECULAR, m_specular);
    aiMat->Get(AI_MATKEY_SHININESS, m_shiniess);

    /*
        Load in textures
    */
    //Syncing the enums between assimp and this structure
    std::unordered_map<ETextureType, aiTextureType> textureTypeMap =
    {
        {ETextureType::diffuse,   aiTextureType::aiTextureType_DIFFUSE},
        {ETextureType::normal,    aiTextureType::aiTextureType_NORMALS},
        {ETextureType::metalness, aiTextureType::aiTextureType_METALNESS},
        {ETextureType::roughness, aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS}
    };

    //For every texturetype: add the texture to the map
    for (auto& type : textureTypeMap)
    {
        aiString path;
        std::string filename = "";

        //Get the filepath from Assimp
        if (AI_SUCCESS == aiMat->GetTexture(type.second, 0, &path))
        {
            std::string filepath = path.C_Str();
            //Split up the string path to only the textures filename
            size_t index = filepath.find_last_of("/\\");
            filename = filepath.substr(index + 1);

            //Get the texture
            //Will be nullptr if it did not exist failed to be created
            m_textures[(uint8_t)type.first] = ResourceManager::GetResource<RTexture>(filename);
        }
    }
    textureTypeMap.clear();
}

bool RMaterial::Create(const std::string& filename)
{
    //Load a mtl file
    
    //Default values
    //Ka = ambient (float3)
    //Kd = diffuse (float3)
    //Ks = specular (float3)
    //Ns = shiniess (float)
    //Tr = transparency = opacity (float)

    //Textures
    //map_Ka = map_Kd = diffuse (standard-texture)
    //bump/norm = normalmap
    //map_Pm = metallic
    //map_Pr = roughness

	return false;
}
