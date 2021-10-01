#pragma once
#include "EnginePCH.h"
#include "RMesh.h"

RMesh::RMesh()
{
    m_meshType   = EMeshType::staticMesh;
    m_material   = nullptr;
    m_indexCount = 0;
}

RMesh::~RMesh()
{
}

bool RMesh::CreateVertexBuffer(const std::vector<simple_vertex_t>& vertices)
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

    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&bufferDesc, &subresData, m_vertexBuffer.GetAddressOf());
    return !FAILED(hr);
}

bool RMesh::CreateIndexBuffer(const std::vector<UINT>& indices)
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
    m_indexCount = (UINT)indices.size();

    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&indexBufferDesc, &subresData, m_indexBuffer.GetAddressOf());
    return !FAILED(hr);
}

void RMesh::Render()
{
    if (m_material)
        m_material->BindMaterial();

	//Draw with indexbuffer
    UINT offset = 0;
    UINT stride = sizeof(simple_vertex_t);
    D3D11Core::Get().DeviceContext()->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    D3D11Core::Get().DeviceContext()->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    D3D11Core::Get().DeviceContext()->DrawIndexed(m_indexCount, 0, 0);

    if (m_material)
        m_material->UnBindMaterial();
}

bool RMesh::Create(const std::string& filename)
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

    //Will not go on if the scene has no meshes
    if (!scene->HasMeshes())
    {
#ifdef _DEBUG
        LOG_WARNING("The scene has no meshes...");
#endif 
        importer.FreeScene();
        return false;
    }

    /*
        Load in material from the first mesh.
        Only supports one material per RMesh for now.
        If more materials needed per RMesh performance will be lower.
        This is because we have to change material between every drawcall
    */
    if (scene->HasMaterials())
    {
        //Get the material-index
        UINT index = scene->mMeshes[0]->mMaterialIndex;
        aiString matName;
        scene->mMaterials[index]->Get(AI_MATKEY_NAME, matName);
        std::string name = matName.C_Str();

        //Create a new material and load it. Add it to manager
        m_material = std::make_shared<RMaterial>();
        if (m_material->Create(scene->mMaterials[index]))
            ResourceManager::Get().AddResource(name, m_material);
    }

    //Skeleton mesh
    /*if (mesh->HasBones())
    {
        m_meshType = EMeshType::skeletalMesh
        //[TODO LATER]
        //Load in all the bones
        //anim_vertex_t vertices
    }*/
    //Else do this below
        //Go through all the vertices

   
    /*
        Load in the mesh and all the submeshes
        Combines all the meshes to one. 
        This is to avoid multiple drawcalls
    */
    std::vector<simple_vertex_t> vertices;
    std::vector<UINT> indices;
    UINT indexOffSet = 0;
    for (UINT m = 0; m < scene->mNumMeshes; m++)
    {
        const aiMesh* aimesh = scene->mMeshes[m];
        
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

                    indices.push_back(face.mIndices[id] + indexOffSet);
                }
            }
        }
        //Adding to the offset
        indexOffSet += (localMaxIndex+1);
    }
    //Have to shrink the vectors
    vertices.shrink_to_fit();
    indices.shrink_to_fit();

    //Create vertex and indexbuffer
    bool success = true;
    if (!CreateVertexBuffer(vertices) ||
        !CreateIndexBuffer(indices))
    {
#ifdef _DEBUG
        LOG_WARNING("Failed to load vertex- or indexbuffer...");
#endif 
        success = false;
    }
    
    //Cleaning
    vertices.clear();
    indices.clear();
    importer.FreeScene();

    std::cout << "Mesh has albedo: " << m_material->HasTexture(ETextureType::albedo) << std::endl;
    std::cout << "Mesh has normal: " << m_material->HasTexture(ETextureType::normal) << std::endl;
    std::cout << "Mesh has roughness: " << m_material->HasTexture(ETextureType::roughness) << std::endl;
    std::cout << "Mesh has metalness: " << m_material->HasTexture(ETextureType::metalness) << std::endl;
    std::cout << "Mesh has ambientOcclusion: " << m_material->HasTexture(ETextureType::ambientOcclusion) << std::endl;
    std::cout << "Mesh has displacement: " << m_material->HasTexture(ETextureType::displacement) << std::endl;
    std::cout << "Mesh has length: " << m_material->HasTexture(ETextureType::length) << std::endl;


    return success;
}
