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

std::string RMaterial::GetFilename(const std::string& path)
{
    //Split up the string path to only the textures filename
    size_t index = path.find_last_of("/\\");
    return path.substr(index + 1);
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
        {ETextureType::albedo,    aiTextureType::aiTextureType_DIFFUSE},
        {ETextureType::normal,    aiTextureType::aiTextureType_NORMALS},
        {ETextureType::metalness, aiTextureType::aiTextureType_METALNESS},
        {ETextureType::roughness, aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS},
        {ETextureType::aoMap,     aiTextureType::aiTextureType_AMBIENT_OCCLUSION}
    };

    //For every texturetype: add the texture to the map
    for (auto& type : textureTypeMap)
    {
        aiString path;
        //Get the filepath from Assimp
        if (AI_SUCCESS == aiMat->GetTexture(type.second, 0, &path))
        {
            std::string filename = GetFilename(path.C_Str());
            //Get the texture
            //Will be nullptr if it did not exist failed to be created
            m_textures[(uint8_t)type.first] = ResourceManager::Get().GetResource<RTexture>(filename);
        }
    }
    textureTypeMap.clear();
}

bool RMaterial::Create(const std::string& filename)
{
    //Load a mtl file
    m_name = filename;
    std::string filepath = "../Assets/Materials/" + filename;
    std::ifstream readfile(filepath);
    
    //File was successfully loaded
    if (readfile.is_open())
    {
        std::string line;
        while (std::getline(readfile, line))
        {
            std::stringstream ss(line);
            std::string prefix;
            ss >> prefix;

            if (prefix == "newmtl")
            {
                ss >> m_name;
            }

            /*
                Basic material information
            */

            //Ambient
            else if (prefix == "Ka")
            {
                ss >> m_ambient.x;
                ss >> m_ambient.y;
                ss >> m_ambient.z;
            }
            //Diffuse
            else if (prefix == "Kd")
            {
                ss >> m_diffuse.x;
                ss >> m_diffuse.y;
                ss >> m_diffuse.z;
            }
            //Specular
            else if (prefix == "Ks")
            {
                ss >> m_specular.x;
                ss >> m_specular.y;
                ss >> m_specular.z;
            }
            //Shiniess
            else if (prefix == "Ns")
            {
                ss >> m_shiniess;
            }
            //Transparency/ opacity
            else if (prefix == "Tr")
            {
                ss >> m_opacity;
            }
            
            /*
                Textures
            */

            //Diffuse/albedo map
            else if (prefix == "map_Ka" || prefix == "map_Kd")
            {
                std::string filepath;
                if (ss >> filepath)
                {
                    std::string filename = GetFilename(filepath);
                    m_textures[(uint8_t)ETextureType::albedo] = ResourceManager::Get().GetResource<RTexture>(filename);
                }
            }
            //Normalmap
            else if (prefix == "bump" || prefix == "norm")
            {
                std::string filepath;
                if (ss >> filepath)
                {
                    std::string filename = GetFilename(filepath);
                    m_textures[(uint8_t)ETextureType::normal] = ResourceManager::Get().GetResource<RTexture>(filename);
                }
            }
            //Metallic
            else if (prefix == "map_Pm" || prefix == "Pm")
            {
                std::string filepath;
                if (ss >> filepath)
                {
                    std::string filename = GetFilename(filepath);
                    m_textures[(uint8_t)ETextureType::metalness] = ResourceManager::Get().GetResource<RTexture>(filename);
                }
            }
            //Roughness
            else if (prefix == "map_Pr" || prefix == "Pr")
            {
                std::string filepath;
                if (ss >> filepath)
                {
                    std::string filename = GetFilename(filepath);
                    m_textures[(uint8_t)ETextureType::roughness] = ResourceManager::Get().GetResource<RTexture>(filename);
                }
            }
            //Ambient occulution map
            else if (prefix == "map_AO" || prefix == "AO")
            {
                std::string filepath;
                if (ss >> filepath)
                {
                    std::string filename = GetFilename(filepath);
                    m_textures[(uint8_t)ETextureType::aoMap] = ResourceManager::Get().GetResource<RTexture>(filename);
                }
            }
        }
        readfile.close();
        return true;

    }
    else
    {
        return false;
    }
}
