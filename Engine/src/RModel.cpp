#pragma once
#include "EnginePCH.h"
#include "RModel.h"

RModel::RModel()
{
    m_hasSkeleton = false;
}

RModel::~RModel()
{
    m_meshes.clear();
    m_allBones.clear();
    m_lights.clear();
}

bool RModel::HasSkeleton() const
{
    return m_hasSkeleton;
}

const std::vector<bone_t>& RModel::GetSkeleton() const
{
    return m_allBones;
}

const std::vector<light_t>& RModel::GetLights() const
{
    return m_lights;
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

const std::string RModel::GetFileFormat(const std::string& filename) const
{
    size_t startIndex = filename.find_last_of(".");
    return filename.substr(startIndex);
}

bool RModel::CombineMeshes(std::vector<aiMesh*>& submeshes, const aiNode* root, submesh_t& submesh)
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
            simpleVertices.push_back(vert);
            
            if (aimesh->HasBones())
            {
                anim_vertex_t animVert = {};
                animVert.vertex = vert;
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
            if (LoadBones(aimesh, root, skeletonVertices))
            {
                m_hasSkeleton = true;
            }
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
        return false;
    }
    indices.clear();

    /*
        The model a skeleton / bones, and therefore we create it with that vector.
        Otherwise we use the regular vertexbuffer.
    */
    if (m_hasSkeleton)
    {
        //Finally create the buffer
        skeletonVertices.shrink_to_fit();
        if (!CreateVertexBuffer(skeletonVertices, submesh))
        {
#ifdef _DEBUG
            LOG_ERROR("Failed to create vertexbuffer for skeleton...");
#endif
            return false;
        }
        else
        {
#ifdef _DEBUG
            std::cout << "Successfully loaded the skeleton!\nIndex\tParentIndex\t\tName" << std::endl;
            for (size_t i = 0; i < m_allBones.size(); i++)
            {
                std::cout << i << "\t" << m_allBones[i].parentIndex << "\t" << m_allBones[i].name << std::endl;
            }
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
            return false;
        }
    }
      
    simpleVertices.clear();
    skeletonVertices.clear();
    return true;
}

bool RModel::CreateVertexBuffer(const std::vector<anim_vertex_t>& vertices, submesh_t& mesh)
{
    D3D11_BUFFER_DESC bufferDesc = {};

    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(anim_vertex_t) * vertices.size());
    bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA subresData = {};

    subresData.pSysMem = &vertices[0];
    subresData.SysMemPitch = 0;
    subresData.SysMemSlicePitch = 0;

    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&bufferDesc, &subresData, mesh.vertexBuffer.GetAddressOf());
    return !FAILED(hr);
}

bool RModel::CreateVertexBuffer(const std::vector<simple_vertex_t>& vertices, submesh_t& mesh)
{
    D3D11_BUFFER_DESC bufferDesc = {};

    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(simple_vertex_t) * vertices.size());
    bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA subresData = {};

    subresData.pSysMem = &vertices[0];
    subresData.SysMemPitch = 0;
    subresData.SysMemSlicePitch = 0;

    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&bufferDesc, &subresData, mesh.vertexBuffer.GetAddressOf());
    return !FAILED(hr);
}

bool RModel::CreateIndexBuffer(const std::vector<UINT>& indices, submesh_t& mesh)
{
    D3D11_BUFFER_DESC indexBufferDesc;
    ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));

    indexBufferDesc.ByteWidth = sizeof(UINT) * (UINT)indices.size();
    indexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    indexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA subresData;
    ZeroMemory(&subresData, sizeof(D3D11_SUBRESOURCE_DATA));

    subresData.pSysMem = &indices[0];
    subresData.SysMemPitch = 0;
    subresData.SysMemSlicePitch = 0;

    mesh.indexCount = (UINT)indices.size();
    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&indexBufferDesc, &subresData, mesh.indexBuffer.GetAddressOf());
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

void RModel::LoadMaterial(const aiScene* scene, const UINT& matIndex, bool& useMTL, submesh_t& inoutMesh) const
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
        if (material->Create(scene->mMaterials[matIndex], useMTL))
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

bool RModel::LoadBones(const aiMesh* aimesh, const aiNode* root, std::vector<anim_vertex_t>& skeletonVertices)
{
    //Data that will be used temporarily for this.
    std::unordered_map<std::string, UINT> nameToIndex;
    std::vector<UINT> boneCounter;
    boneCounter.resize(skeletonVertices.size(), 0);

    //OffSet if the mesh has multiple armatures
    UINT boneOffSet = 0;
    if (!m_allBones.empty())
        boneOffSet = static_cast<UINT>(m_allBones.size());

    //Go through all the bones
    for (UINT b = 0; b < aimesh->mNumBones; b++)
    {
        const aiBone* aibone = aimesh->mBones[b];
        nameToIndex[aibone->mName.C_Str()] = b + boneOffSet;

        //Go through all the vertices that the bone affect
        for (UINT v = 0; v < aibone->mNumWeights; v++)
        {
            UINT id         = aibone->mWeights[v].mVertexId;
            float weight    = aibone->mWeights[v].mWeight;

            boneCounter[id]++;
            switch (boneCounter[id])
            {
            case 1:
                skeletonVertices[id].boneIDs.x      = b + boneOffSet;
                skeletonVertices[id].boneWeights.x  = weight;
                break;
            case 2:
                skeletonVertices[id].boneIDs.y      = b + boneOffSet;
                skeletonVertices[id].boneWeights.y  = weight;
                break;
            case 3:
                skeletonVertices[id].boneIDs.z      = b + boneOffSet;
                skeletonVertices[id].boneWeights.z  = weight;
                break;
            case 4:
                skeletonVertices[id].boneIDs.w      = b + boneOffSet;
                skeletonVertices[id].boneWeights.w  = weight;
                break;
            default:
                //Vertex already has 4 bones connected
                break;
            };
        }

        //Normalize the values - have to sum up to 1.0
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

        //Create the bone and find the parent index
        bone_t bone;
        bone.name = aibone->mName.C_Str();
        bone.inverseBind = sm::Matrix(&aibone->mOffsetMatrix.a1);
        std::string parentName = root->FindNode(bone.name.c_str())->mParent->mName.C_Str();
        
        //Root should not add a parent
        if (nameToIndex.find(parentName) != nameToIndex.end())
            bone.parentIndex = static_cast<int>(nameToIndex[parentName]);

        m_allBones.push_back(bone);
    }
    m_allBones.shrink_to_fit();

    //Freeing up space
    nameToIndex.clear();
    boneCounter.clear();

    return true;
}

void RModel::Render() const
{
    //Bind vertexshader
    //Bind inputlayout
    //D3D11Core::Get().DeviceContext()->VSSetShader(m_vertexShader->Get(), 0, 0);

    UINT offset = 0;
    UINT stride = sizeof(simple_vertex_t);
    if (m_hasSkeleton)
        stride = sizeof(anim_vertex_t);

    for (size_t m = 0; m < m_meshes.size(); m++)
    {
        if (m_meshes[m].material)
            m_meshes[m].material->BindMaterial();
        
        D3D11Core::Get().DeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        D3D11Core::Get().DeviceContext()->IASetVertexBuffers(0, 1, m_meshes[m].vertexBuffer.GetAddressOf(), &stride, &offset);
        D3D11Core::Get().DeviceContext()->IASetIndexBuffer(m_meshes[m].indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        D3D11Core::Get().DeviceContext()->DrawIndexed(m_meshes[m].indexCount, 0, 0);

		if (m_meshes[m].material)
            m_meshes[m].material->UnBindMaterial();
    }


    //Unbind vertexshader
    //Unbind inputlayout
}

void RModel::RenderDeferred(ID3D11DeviceContext* context)
{
    UINT offset = 0;
    UINT stride = sizeof(simple_vertex_t);
    if (m_hasSkeleton)
        stride = sizeof(anim_vertex_t);

    for (size_t m = 0; m < m_meshes.size(); m++)
    {
        if (m_meshes[m].material)
            m_meshes[m].material->BindDeferredMaterial(context);

        context->IASetVertexBuffers(0, 1, m_meshes[m].vertexBuffer.GetAddressOf(), &stride, &offset);
        context->IASetIndexBuffer(m_meshes[m].indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        context->DrawIndexed(m_meshes[m].indexCount, 0, 0);

        if (m_meshes[m].material)
            m_meshes[m].material->UnBindDeferredMaterial(context);
    }
}

bool RModel::Create(const std::string& filename)
{
    std::string filepath = MODELPATH + filename;
    Assimp::Importer importer;

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
    std::unordered_map<UINT, std::vector<aiMesh*>> matSet;
    for (UINT i = 0; i < scene->mNumMeshes; i++)
    {
        UINT matIndex = scene->mMeshes[i]->mMaterialIndex;
        matSet[matIndex].push_back(scene->mMeshes[i]);
    }

    //.fbx uses default material, and .obj uses mtl
    bool useMTL = false;
    if (GetFileFormat(filename) == ".obj")
        useMTL = true;

    /*
        Loads in each material and then combine
        the multiple meshes to one, if needed.
    */
    for (auto& mat : matSet)
    {
        submesh_t submesh;
        
        //Load in the material at index and with mtl-format or default
        LoadMaterial(scene, mat.first, useMTL, submesh);

        /*
            Load in vertex- and index-data and combines all the meshes in a set to one
        */
        if (!CombineMeshes(mat.second, scene->mRootNode, submesh))
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
    importer.FreeScene();
    return true;
}
