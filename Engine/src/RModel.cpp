#include "EnginePCH.h"
#include "RModel.h"

RModel::RModel()
{
}

RModel::~RModel()
{
    m_meshes.clear();
    m_lights.clear();
    m_allBones.clear();
}

bool RModel::HasSkeleton() const
{
    return !m_allBones.empty();
}

const std::vector<bone_t>& RModel::GetSkeleton() const
{
    return m_allBones;
}

const std::vector<light_t>& RModel::GetLights() const
{
    return m_lights;
}

const std::vector<Mesh>& RModel::GetMeshes() const
{
    return m_meshes;
}

bool RModel::ChangeMaterial(const std::string& mtlfile)
{
    const std::string filepath = MATERIALPATH + mtlfile;
    std::ifstream readfile(filepath);
    if (!readfile.is_open())
        return false;
    /*
        Load data from file
    */
    std::string line;
    std::string totalData = "";
    while (std::getline(readfile, line))
    {
        if (!line.empty())
        {
            line += '\n';
            totalData.append(line);
        }
    }
    readfile.close();
    std::vector<std::string> blocks;
    size_t mtlIndex = -1;
    bool hasReachedEnd = false;
    const std::string newmtl = "newmtl";
    /*
        Split the total data into multiple blocks
    */
    while (!hasReachedEnd)
    {
        mtlIndex = totalData.find(newmtl, mtlIndex + 1);
        if (mtlIndex != std::string::npos)
        {
            size_t mtlLast = totalData.find(newmtl, mtlIndex + 1);
            blocks.push_back(totalData.substr(mtlIndex, mtlLast - mtlIndex));
        }
        else
            hasReachedEnd = true;
    }
    /*
        Get the names of all the materials
    */
    std::vector<std::string> materialNames;
    for (int i = 0; i < blocks.size(); i++)
    {
        size_t nameIndex = blocks[i].find(newmtl) + newmtl.length() + 1;
        size_t nameEnd = blocks[i].find('\n');
        std::string matName = blocks[i].substr(nameIndex, nameEnd - nameIndex);
        materialNames.push_back(matName);
    }
    /*
        Bind the "new" materials to the model
    */
    for (size_t m = 0; m < m_meshes.size(); m++)
    {
        if (m < materialNames.size())
        {
            const std::string matName = materialNames[m];
            if (!m_meshes[m].SetMaterial(matName))
            {
                m_meshes[m].SetMaterialFromMTL(blocks[m]);
            }
        }
    }
    blocks.clear();
    materialNames.clear();
    return true;
}

bool RModel::CreateOneMesh(aiMaterial* aimat, aiMesh* aimesh, const std::unordered_map<std::string, UINT>& boneMap)
{
    Mesh mesh;
    if (!mesh.SetMaterial(aimat))
    {
        LOG_WARNING("Failed to set material");
    }

    if (!mesh.Create(aimesh, boneMap))
    {
        LOG_WARNING("Failed to create mesh");
        return false;
    }

    m_meshes.push_back(mesh);
    return true;
}

bool RModel::CombineMeshes(aiMaterial* aimat, std::vector<aiMesh*>& aimeshes, const std::unordered_map<std::string, UINT>& boneMap)
{
    Mesh mesh;
    if (!mesh.SetMaterial(aimat))
    {
        LOG_WARNING("Failed to set material");
    }

    if (!mesh.Create(aimeshes, boneMap))
    {
        LOG_WARNING("Failed to create mesh");
        return false;
    }

    m_meshes.push_back(mesh);
    return true;
}

void RModel::LoadLights(const aiScene* scene)
{
    for (UINT i = 0; i < scene->mNumLights; i++)
    {
        const aiLight* ailight = scene->mLights[i];
        light_t light;

        aiVector3D aiRot, aiScl, aiPos;
        scene->mRootNode->FindNode(ailight->mName)->mTransformation.Decompose(aiScl, aiRot, aiPos);
        light.position  = { aiPos.x + ailight->mPosition.x, aiPos.y + ailight->mPosition.y, aiPos.z + ailight->mPosition.z, 0.0f };
        light.direction = { ailight->mDirection.x,          ailight->mDirection.y,          ailight->mDirection.z,          0.0f };
        light.color     = { ailight->mColorDiffuse.r,       ailight->mColorDiffuse.g,       ailight->mColorDiffuse.b,       0.0f };

        if (ailight->mType == aiLightSourceType::aiLightSource_POINT)
        {
            light.type = TypeLight::POINT;
        }
        else if (ailight->mType == aiLightSourceType::aiLightSource_DIRECTIONAL)
        {
            light.type = TypeLight::DIRECTIONAL;
        }
        light.enabled = true;
        m_lights.push_back(light);
    }
    m_lights.shrink_to_fit();
}

void RModel::BoneHierchy(aiNode* node, std::unordered_map<std::string, bone_t>& nameToBone)
{
    //Check if the bone exist in the map
    auto boneIterator = nameToBone.find(node->mName.C_Str());
    if (boneIterator != nameToBone.end())
    {
        m_allBones.push_back(boneIterator->second);
    }

    //Check the children
    for (UINT i = 0; i < node->mNumChildren; i++)
    {
        BoneHierchy(node->mChildren[i], nameToBone);
    }
}

void RModel::Render(ID3D11DeviceContext* context)
{
    for (size_t i = 0; i < m_meshes.size(); i++)
    {
        m_meshes[i].Render(context);
    }
}

bool RModel::Create(const std::string& filename)
{
    std::string filepath = MODELPATH + filename;
    Assimp::Importer importer;

    //Will remove extra text on bones like: "_$AssimpFbx$_"...
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    
    const aiScene* scene = importer.ReadFile
    (
        filepath,
        aiProcess_Triangulate           |   //Triangulate every surface
        aiProcess_JoinIdenticalVertices |   //Ignores identical veritices - memory saving  
        aiProcess_FlipWindingOrder      |   //Makes it clockwise order
        aiProcess_MakeLeftHanded        |	//Use a lefthanded system for the models 
        aiProcess_CalcTangentSpace      |   //Fix tangents and bitangents automatic for us
        aiProcess_FlipUVs               |   //Flips the textures to fit directX-style
        aiProcess_LimitBoneWeights 		    //Limits by default to 4 weights per vertex
    );

    //Check if readfile was successful
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
#ifdef _DEBUG
        LOG_WARNING("Assimp error: %s", importer.GetErrorString());
#endif 
        importer.FreeScene();
        return false;
    }

    /*
        The model has to have meshes for this to work
    */
    if (!scene->HasMeshes())
    {
#ifdef _DEBUG
        LOG_WARNING("The model has no meshes...");
#endif 
        importer.FreeScene();
        return false;
    }

    LoadLights(scene);

   
    /*
        Links together a material to multiple submeshes - a set
    */
    std::unordered_map<std::string, bone_t> nameToBone;
    std::unordered_map<UINT, std::vector<aiMesh*>> matSet;
    for (UINT i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh* aimesh = scene->mMeshes[i];
        UINT matIndex = aimesh->mMaterialIndex;
        matSet[matIndex].push_back(aimesh);

        //Add bones if they exist
        for (UINT b = 0; b < aimesh->mNumBones; b++)
        {
            const aiBone* aibone = aimesh->mBones[b];
            std::string boneName = aibone->mName.C_Str();

            //Bone does not exist - create it
            if (nameToBone.find(boneName) == nameToBone.end())
            {
                bone_t bone;
                bone.name = boneName;
                bone.inverseBind = sm::Matrix(&aibone->mOffsetMatrix.a1).Transpose();
                nameToBone[boneName] = bone;
            }
        }
    }

    /*
        Add all the bones that we found. Link together who the parent is.
    */
    std::unordered_map<std::string, UINT> boneMap;
    BoneHierchy(scene->mRootNode, nameToBone);
    for (UINT i = 0; i < m_allBones.size(); i++)
    {
        if (boneMap.find(m_allBones[i].name) == boneMap.end())
        {
            boneMap[m_allBones[i].name] = i;
        }

        std::string parentName = scene->mRootNode->FindNode(m_allBones[i].name.c_str())->mParent->mName.C_Str();
        if (boneMap.find(parentName) != boneMap.end())
            m_allBones[i].parentIndex = boneMap[parentName];
       
//#ifdef _DEBUG
//        std::cout << i << "\t" << m_allBones[i].parentIndex << "\t" << m_allBones[i].name << std::endl;
//#endif // _DEBUG
    }

    /*
        Loads in each material and then combine
        the multiple meshes to one, if needed.
    */
    for (auto& mat : matSet)
    {
        if (OPTIMIZE_MODEL)
        {
            if (!CombineMeshes(scene->mMaterials[mat.first], mat.second, boneMap))
            {
                importer.FreeScene();
                return false;
            }
        }
        //Load in each aimesh as one mesh
        else
        {
            for (int i = 0; i < mat.second.size(); i++)
            {
                if (!CreateOneMesh(scene->mMaterials[mat.first], mat.second[i], boneMap))
                {
                    importer.FreeScene();
                    return false;
                }
            }
        }
    }
    m_meshes.shrink_to_fit();

    //Freeing memory
    matSet.clear();
    boneMap.clear();
    importer.FreeScene();
    return true;
}
