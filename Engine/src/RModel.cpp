#pragma once
#include "EnginePCH.h"
#include "RModel.h"

RModel::RModel()
{
}

RModel::~RModel()
{
    m_meshes.clear();
    m_materials.clear();
}

bool RModel::ChangeMaterial(const std::string& mtlfile)
{
    /*
        The new material have to have as many submaterials as previously,
        otherwise things will look weird...

        Load in the material from a mtl
        * Can have many materials in same file
        * 

    */
    return false;
}

const std::string RModel::GetFileFormat(const std::string& filename) const
{
    size_t startIndex = filename.find_last_of(".");
    return filename.substr(startIndex);
}

bool RModel::CombineMeshes(std::vector<aiMesh*>& submeshes)
{
    std::vector<simple_vertex_t> vertices;
    std::vector<UINT> indices;
    //Offset when moving between different submeshes
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
            vertices.push_back(vert);
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
    }

    //Have to shrink the vectors
    vertices.shrink_to_fit();
    indices.shrink_to_fit();

    //Create a new mesh
    mesh_t newMesh;

    //Create vertex and indexbuffer
    bool success = true;
    if (!CreateVertexBuffer(vertices, newMesh) ||
        !CreateIndexBuffer(indices, newMesh))
    {
#ifdef _DEBUG
        LOG_WARNING("Failed to load vertex- or indexbuffer...");
#endif
        success = false;
    }

    //Cleaning
    vertices.clear();
    indices.clear();

    if (success)
    {
        //Finally add the mesh to the vector
        m_meshes.push_back(newMesh);
        return true;
    }
    else
    {
        return false;
    }
}

bool RModel::CreateVertexBuffer(const std::vector<simple_vertex_t>& vertices, mesh_t& mesh)
{
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));

    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(simple_vertex_t) * vertices.size());
    bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA subresData;
    ZeroMemory(&subresData, sizeof(D3D11_SUBRESOURCE_DATA));

    subresData.pSysMem = &vertices[0];
    subresData.SysMemPitch = 0;
    subresData.SysMemSlicePitch = 0;

    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&bufferDesc, &subresData, mesh.vertexBuffer.GetAddressOf());
    return !FAILED(hr);
}

bool RModel::CreateIndexBuffer(const std::vector<UINT>& indices, mesh_t& mesh)
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

void RModel::Render() const
{
    UINT offset = 0;
    UINT stride = sizeof(simple_vertex_t);
    for (size_t m = 0; m < m_meshes.size(); m++)
    {
        if (m_materials[m])
            m_materials[m]->BindMaterial();
        
        D3D11Core::Get().DeviceContext()->IASetVertexBuffers(0, 1, m_meshes[m].vertexBuffer.GetAddressOf(), &stride, &offset);
        D3D11Core::Get().DeviceContext()->IASetIndexBuffer(m_meshes[m].indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        D3D11Core::Get().DeviceContext()->DrawIndexed(m_meshes[m].indexCount, 0, 0);
        
        if (m_materials[m])
            m_materials[m]->UnBindMaterial();
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
        aiProcess_FlipUVs                   //Flips the textures to fit directX-style
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
        The model has to have meshes and materials for this to work
    */
    if (!scene->HasMeshes())
    {
#ifdef _DEBUG
        LOG_WARNING("The model has no meshes...");
#endif 
        importer.FreeScene();
        return false;
    }
    if (!scene->HasMaterials())
    {
#ifdef _DEBUG
        LOG_WARNING("The model has no materials...");
#endif 
        importer.FreeScene();
        return false;
    }

    /*
        Testing AABB
    */
    //vector3 min = scene->mMeshes[0]->mAABB.mMin
    //vector3 max = scene->mMeshes[0]->mAABB.mMax
    //scene->mMeshes[0]->mAABB.mMax
    //DirectX::BoundingBox::CreateFromPoints(boundingbox, min, max)
    /*
        IDEA:
        for every mesh
            load in max and min
            create a boundingbox
            pushback to the vector

        An idea too:
            A submesh can have multiple colliders
            This is because when combining meshes we lose precion

        Collidercheck
            bouningbox RModel::GetCollider() const
            bool Collided(const RModel*  mesh) const
                first check the big boundingbox
                    if false we ignore it from here.
                    else we shall check deeper and see which one that collided with

    */


    std::string fileformat = GetFileFormat(filename);

    /*
        Loading submeshes and multiple materials
    */
    std::unordered_map<UINT, std::vector<aiMesh*>> matSet;
    //Creating all the sets
    for (UINT i = 0; i < scene->mNumMeshes; i++)
    {
        UINT matIndex = scene->mMeshes[i]->mMaterialIndex;
        matSet[matIndex].push_back(scene->mMeshes[i]);
    }

    //Load in every material needed
    for (auto& mat : matSet)
    {
        UINT matIndex = mat.first;
        //Get name of the material
        aiString matName;
        scene->mMaterials[matIndex]->Get(AI_MATKEY_NAME, matName);

        //Load in the materials
        std::shared_ptr<RMaterial> material = std::make_shared<RMaterial>();
        if (material->Create(scene->mMaterials[matIndex], fileformat))
        {
            ResourceManager::Get().AddResource(matName.C_Str(), material);
            m_materials.push_back(material);
        }

        /*
            Load in vertex- and index-data and combines all the meshes in a set to one
        */
        if (!CombineMeshes(mat.second))
        {
            importer.FreeScene();
            return false;
        }
    }

    matSet.clear(); 
    importer.FreeScene();
    return true;
}
