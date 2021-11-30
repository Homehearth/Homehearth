#pragma once
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

const std::vector<sm::Vector2> RModel::GetTextureCoords() const
{
    std::vector<sm::Vector2> texturecoords;

    ComPtr<ID3D11Buffer> tempResource;
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    //Go through all the meshes
    for (size_t i = 0; i < m_meshes.size(); i++)
    {
        UINT byteWidth = 0;

        if (m_meshes[i].hasBones)
            byteWidth = static_cast<UINT>(sizeof(anim_vertex_t) * m_meshes[i].vertexCount);
        else
            byteWidth = static_cast<UINT>(sizeof(simple_vertex_t) * m_meshes[i].vertexCount);

        desc.ByteWidth = byteWidth;

        HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&desc, nullptr, tempResource.GetAddressOf());
        if (FAILED(hr))
        {
#ifdef _DEBUG
            LOG_ERROR("Failed to create staging buffer for get texturecoords...");
#endif // _DEBUG
            return texturecoords;
        }

        //Copy the resource to staging buffer so that we can read it
        D3D11Core::Get().DeviceContext()->CopyResource(tempResource.Get(), m_meshes[i].vertexBuffer.Get());

        std::vector<simple_vertex_t> simpleVertices;
        std::vector<anim_vertex_t>   animVertices;

        //Copy the data
        D3D11_MAPPED_SUBRESOURCE data;
        D3D11Core::Get().DeviceContext()->Map(tempResource.Get(), 0, D3D11_MAP_READ, 0, &data);

        if (m_meshes[i].hasBones)
        {
            animVertices.resize(m_meshes[i].vertexCount);
            memcpy(&animVertices[0], data.pData, byteWidth);

            //Go through the vector of vertices and get the texturecoords
            for (size_t v = 0; v < animVertices.size(); v++)
            {
                texturecoords.push_back(animVertices[v].uv);
            }
            animVertices.clear();
        }
        else
        {
            simpleVertices.resize(m_meshes[i].vertexCount);
            memcpy(&simpleVertices[0], data.pData, byteWidth);

            //Go through the vector of vertices and get the texturecoords
            for (size_t v = 0; v < simpleVertices.size(); v++)
            {
                texturecoords.push_back(simpleVertices[v].uv);
            }
            simpleVertices.clear();
        }
        D3D11Core::Get().DeviceContext()->Unmap(tempResource.Get(), 0);
    }

    return texturecoords;
}

const std::vector<std::shared_ptr<RTexture>> RModel::GetTextures(const ETextureType& type) const
{
    std::vector<std::shared_ptr<RTexture>> textures;

    for (size_t i = 0; i < m_meshes.size(); i++)
    {
        textures.push_back(m_meshes[i].material->GetTexture(type));
    }

    return textures;
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
        mtlIndex = totalData.find(newmtl, mtlIndex +1);
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
        m_meshes[m].material.reset();
        if (m < materialNames.size())
        {
            const std::string matName = materialNames[m];
            m_meshes[m].material = ResourceManager::Get().GetResource<RMaterial>(matName);
            
            //Material not found - going to try to create it
            if (!m_meshes[m].material)
            {
                m_meshes[m].material = std::make_shared<RMaterial>();
                if (m_meshes[m].material->CreateFromMTL(blocks[m]))
                {
                    ResourceManager::Get().AddResource(matName, m_meshes[m].material);
                }
            }
        }
    }
    blocks.clear();
    materialNames.clear();
    return true;
}

bool RModel::CombineMeshes(std::vector<aiMesh*>& submeshes, submesh_t& submesh, const std::unordered_map<std::string, UINT>& boneMap)
{
    std::vector<simple_vertex_t> simpleVertices;
    std::vector<anim_vertex_t> skeletonVertices;
    std::vector<UINT> indices;
    //Offset for indices when moving between different submeshes
    UINT indexOffset = 0;

    //Go through all the meshes
    for (UINT m = 0; m < submeshes.size(); m++)
    {
        const aiMesh* aimesh = submeshes[m];

        //Load in vertices
        for (UINT v = 0; v < aimesh->mNumVertices; v++)
        {
            simple_vertex_t vert = {};
            vert.position   = { aimesh->mVertices[v].x,         aimesh->mVertices[v].y,       aimesh->mVertices[v].z    };
            vert.uv         = { aimesh->mTextureCoords[0][v].x, aimesh->mTextureCoords[0][v].y                          };
            vert.normal     = { aimesh->mNormals[v].x,          aimesh->mNormals[v].y,        aimesh->mNormals[v].z     };
            vert.tangent    = { aimesh->mTangents[v].x,         aimesh->mTangents[v].y,       aimesh->mTangents[v].z    };
            vert.bitanget   = { aimesh->mBitangents[v].x,       aimesh->mBitangents[v].y,     aimesh->mBitangents[v].z  };
            
            //Only accept the first colorset for now
            if (aimesh->HasVertexColors(0))
                vert.color  = { aimesh->mColors[0][v].r,        aimesh->mColors[0][v].g,      aimesh->mColors[0][v].b   };
            simpleVertices.push_back(vert);
            
            if (aimesh->HasBones())
            {
                anim_vertex_t animVert = {};
                animVert.position   = vert.position;
                animVert.uv         = vert.uv;
                animVert.normal     = vert.normal;
                animVert.tangent    = vert.tangent;
                animVert.bitanget   = vert.bitanget;
                animVert.color      = vert.color;
                skeletonVertices.push_back(animVert);
            }
        }

        //Max index so that we know how much to step to reach next vertices-set
        UINT localMaxIndex = 0;
        //Going through all the indices
        for (UINT f = 0; f < aimesh->mNumFaces; f++)
        {
            const aiFace face = aimesh->mFaces[f];
            if (face.mNumIndices == 3)
            {
                for (unsigned int id = 0; id < 3; id++)
                {
                    UINT faceIndex = face.mIndices[id];
                    if (faceIndex > localMaxIndex)
                        localMaxIndex = faceIndex;

                    indices.push_back(face.mIndices[id] + indexOffset);
                }
            }
        }
        //Adding to the offset
        indexOffset += (localMaxIndex+1);

        //Load in the skeleton if it exist
        if (aimesh->HasBones())
        {
            skeletonVertices.shrink_to_fit();
            LoadVertexSkinning(aimesh, skeletonVertices, boneMap);
            submesh.hasBones = true;
        }
    }

    /*
        Creating buffers
    */
    indices.shrink_to_fit();
    if (!CreateIndexBuffer(indices, submesh))
    {
#ifdef _DEBUG
        LOG_ERROR("Failed to create indexbuffer...");
#endif
    }
    indices.clear();

    /*
        The model a skeleton / bones, and therefore we create it with that vector.
        Otherwise we use the regular vertexbuffer.
    */
    if (submesh.hasBones)
    {
        //Finally create the buffer
        skeletonVertices.shrink_to_fit();
        if (!CreateVertexBuffer(skeletonVertices, submesh))
        {
#ifdef _DEBUG
            LOG_ERROR("Failed to create vertexbuffer for skeleton...");
#endif
        }
    }
    else
    {
        simpleVertices.shrink_to_fit();
        if (!CreateVertexBuffer(simpleVertices, submesh))
        {
#ifdef _DEBUG
            LOG_ERROR("Failed to create vertexbuffer...");
#endif
        }
    }
      
    simpleVertices.clear();
    skeletonVertices.clear();
    return true;
}

bool RModel::CreateVertexBuffer(const std::vector<anim_vertex_t>& vertices, submesh_t& mesh)
{
    if (vertices.empty())
        return false;

    D3D11_BUFFER_DESC desc        = {};
    desc.ByteWidth                = static_cast<UINT>(sizeof(anim_vertex_t) * vertices.size());
    desc.Usage                    = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    desc.BindFlags                = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags           = 0;
    desc.MiscFlags                = 0;
    desc.StructureByteStride      = 0;

    D3D11_SUBRESOURCE_DATA data   = {};
    data.pSysMem                  = &vertices[0];
    data.SysMemPitch              = 0;
    data.SysMemSlicePitch         = 0;

    mesh.vertexCount = static_cast<UINT>(vertices.size());
    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&desc, &data, mesh.vertexBuffer.GetAddressOf());
    return !FAILED(hr);
}

bool RModel::CreateVertexBuffer(const std::vector<simple_vertex_t>& vertices, submesh_t& mesh)
{
    if (vertices.empty())
        return false;

    D3D11_BUFFER_DESC desc        = {};
    desc.ByteWidth                = static_cast<UINT>(sizeof(simple_vertex_t) * vertices.size());
    desc.Usage                    = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    desc.BindFlags                = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags           = 0;
    desc.MiscFlags                = 0;
    desc.StructureByteStride      = 0;

    D3D11_SUBRESOURCE_DATA data   = {};
    data.pSysMem                  = &vertices[0];
    data.SysMemPitch              = 0;
    data.SysMemSlicePitch         = 0;

    mesh.vertexCount = static_cast<UINT>(vertices.size());
    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&desc, &data, mesh.vertexBuffer.GetAddressOf());
    return !FAILED(hr);
}

bool RModel::CreateIndexBuffer(const std::vector<UINT>& indices, submesh_t& mesh)
{
    if (indices.empty())
        return false;

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth              = static_cast<UINT>(sizeof(UINT) * indices.size());
    desc.Usage                  = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    desc.BindFlags              = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
    desc.CPUAccessFlags         = 0;
    desc.MiscFlags              = 0;

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem                = &indices[0];
    data.SysMemPitch            = 0;
    data.SysMemSlicePitch       = 0;

    mesh.indexCount = static_cast<UINT>(indices.size());
    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&desc, &data, mesh.indexBuffer.GetAddressOf());
    return !FAILED(hr);
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

void RModel::LoadMaterial(const aiScene* scene, const UINT& matIndex, submesh_t& inoutMesh) const
{
    //Get name of the material
    aiString matName;
    scene->mMaterials[matIndex]->Get(AI_MATKEY_NAME, matName);

    //Check if the material exists
    std::shared_ptr<RMaterial> material = ResourceManager::Get().GetResource<RMaterial>(matName.C_Str(), false);
    if (!material)
    {
        material = std::make_shared<RMaterial>();
        //Add the material to the resourcemanager if it was successfully created
        if (material->Create(scene->mMaterials[matIndex]))
        {
            ResourceManager::Get().AddResource(matName.C_Str(), material);
            inoutMesh.material = material;
        }
        //Else - failed an therefore we shall not link it to the submesh
    }
    else
    {
        inoutMesh.material = material;
    }
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

bool RModel::LoadVertexSkinning(const aiMesh* aimesh, std::vector<anim_vertex_t>& skeletonVertices, const std::unordered_map<std::string, UINT>& boneMap)
{
    //Data that will be used temporarily for this.
    std::vector<UINT> boneCounter;
    boneCounter.resize(skeletonVertices.size(), 0);

    //Go through all the bones
    for (UINT b = 0; b < aimesh->mNumBones; b++)
    {
        aiBone* aibone = aimesh->mBones[b];
        UINT boneNr = boneMap.at(aibone->mName.C_Str());

        //Go through all the vertices that the bone affect
        for (UINT v = 0; v < aibone->mNumWeights; v++)
        {
            UINT id         = aibone->mWeights[v].mVertexId;
            float weight    = aibone->mWeights[v].mWeight;

            switch (boneCounter[id]++)
            {
            case 0:
                skeletonVertices[id].boneIDs.x      = boneNr;
                skeletonVertices[id].boneWeights.x  = weight;
                break;
            case 1:
                skeletonVertices[id].boneIDs.y      = boneNr;
                skeletonVertices[id].boneWeights.y  = weight;
                break;
            case 2:
                skeletonVertices[id].boneIDs.z      = boneNr;
                skeletonVertices[id].boneWeights.z  = weight;
                break;
            case 3:
                skeletonVertices[id].boneIDs.w      = boneNr;
                skeletonVertices[id].boneWeights.w  = weight;
                break;
            default:
                //Vertex already has 4 bones connected
                break;
            };
        }
    }
    m_allBones.shrink_to_fit();

    //Normalize the weights - have to sum up to 1.0
    for (size_t i = 0; i < skeletonVertices.size(); i++)
    {
        sm::Vector4 weights = skeletonVertices[i].boneWeights;
        //Total weight
        float total = weights.x + weights.y + weights.z + weights.w;
        //Avoid devide by 0
        if (total > 0.0f)
        {
            skeletonVertices[i].boneWeights = { weights.x / total,
                                                weights.y / total,
                                                weights.z / total,
                                                weights.w / total };
        }
    }

    //Freeing up space
    boneCounter.clear();
    return true;
}

void RModel::Render(ID3D11DeviceContext* context)
{
    UINT offset = 0;
    UINT stride = 0;

    for (size_t m = 0; m < m_meshes.size(); m++)
    {
        if (m_meshes[m].material)
            m_meshes[m].material->BindMaterial(context);
        
        if (m_meshes[m].hasBones)
            stride = sizeof(anim_vertex_t);
        else
            stride = sizeof(simple_vertex_t);

        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->IASetVertexBuffers(0, 1, m_meshes[m].vertexBuffer.GetAddressOf(), &stride, &offset);
        context->IASetIndexBuffer(m_meshes[m].indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        context->DrawIndexed(m_meshes[m].indexCount, 0, 0);

		if (m_meshes[m].material)
            m_meshes[m].material->UnBindMaterial(context);
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
        submesh_t submesh;
        
        //Load in the material at index
        LoadMaterial(scene, mat.first, submesh);

        /*
            Load in vertex- and index-data and combines all the meshes in a set to one
        */
        if (!CombineMeshes(mat.second, submesh, boneMap))
        {
            importer.FreeScene();
            return false;
        }

        //Add the submesh to the vector
        m_meshes.push_back(submesh);
    }
    m_meshes.shrink_to_fit();

    //Freeing memory
    matSet.clear();
    boneMap.clear();
    importer.FreeScene();
    return true;
}
