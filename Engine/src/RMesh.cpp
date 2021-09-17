#pragma once
#include "EnginePCH.h"
#include "RMesh.h"

RMesh::RMesh()
{
    m_meshType = EMeshType::staticMesh;
    m_material = nullptr;
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

    ComPtr<ID3D11Buffer> vertexBuffer;

    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&bufferDesc, &subresData, vertexBuffer.GetAddressOf());
    if (!FAILED(hr))
    {
        m_meshInfo.vertexBuffer = vertexBuffer;
        return true;
    }
    else
    {
        return false;
    }
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

    ComPtr<ID3D11Buffer> indexBuffer;

    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&indexBufferDesc, &subresData, indexBuffer.GetAddressOf());
    if (!FAILED(hr))
    {
        m_meshInfo.indexBuffer = indexBuffer;
        m_meshInfo.indexCount = (UINT)indices.size();
        return true;
    }
    else
    {
        return false;
    }
}

void RMesh::Render()
{
	//uint16_t currentMat = -1;
    UINT stride = sizeof(simple_vertex_t);
    UINT offset = 0;

    //Upload the material to gpu
    /*
    if (m_material)
    {
        m_material->UploadToGPU();
    }
    */

	//Draw with indexbuffer
    D3D11Core::Get().DeviceContext()->IASetVertexBuffers(0, 1, m_meshInfo.vertexBuffer.GetAddressOf(), &stride, &offset);
    D3D11Core::Get().DeviceContext()->IASetIndexBuffer(m_meshInfo.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    D3D11Core::Get().DeviceContext()->DrawIndexed(m_meshInfo.indexCount, 0, 0);
}

bool RMesh::Create(const std::string& filename)
{
    std::string filepath = "../Assets/Models/" + filename;
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
        std::string errorString = "Assimp error: " + (std::string)importer.GetErrorString();
        LOG_WARNING(errorString.c_str());
        importer.FreeScene();
        return false;
    }

    //Will not go on if the scene has no meshes
    if (!scene->HasMeshes())
    {
        std::string errorString = "The scene has no meshes...";
        LOG_WARNING(errorString.c_str());
        importer.FreeScene();
        return false;
    }

    //Get the name of the material
    if (scene->HasMaterials())
    {
        aiString matName;
        scene->mMaterials[0]->Get(AI_MATKEY_NAME, matName);
        std::string name = matName.C_Str();
        
        //Search and see if the material already has been loaded.
        //If it was not loaded, we try to create it by searching the mtl-files
        m_material = ResourceManager::GetResource<RMaterial>(name);

        //Material was not loaded before
        if (!m_material)
        {
            m_material->LoadMaterial(scene->mMaterials[0]);
            ResourceManager::InsertResource(name, m_material);
        }
    }

    //Load in the mesh
    const aiMesh* aimesh = scene->mMeshes[0];

    std::vector<simple_vertex_t> vertices;
    std::vector<UINT> indices;
    vertices.reserve(aimesh->mNumVertices);
    indices.reserve(size_t(aimesh->mNumFaces) * 3);

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

    for (unsigned int v = 0; v < aimesh->mNumVertices; v++)
    {
        simple_vertex_t vert = {};
        vert.position = { aimesh->mVertices[v].x,         aimesh->mVertices[v].y,       aimesh->mVertices[v].z  };
        vert.uv       = { aimesh->mTextureCoords[0][v].x, aimesh->mTextureCoords[0][v].y                        };
        vert.normal   = { aimesh->mNormals[v].x,          aimesh->mNormals[v].y,        aimesh->mNormals[v].z   };
        vert.tangent  = { aimesh->mTangents[v].x,         aimesh->mTangents[v].y,       aimesh->mTangents[v].z  };
        vert.bitanget = { aimesh->mBitangents[v].x,       aimesh->mBitangents[v].y,     aimesh->mBitangents[v].z};
        vertices.push_back(vert);
    }

    //Indices
    for (unsigned int f = 0; f < aimesh->mNumFaces; f++)
    {
        const aiFace face = aimesh->mFaces[f];
        if (face.mNumIndices == 3)
        {
            for (unsigned int id = 0; id < 3; id++)
                indices.push_back(face.mIndices[id]);
        }
    }

    bool success = true;
    //Create vertex and indexbuffer
    if (!CreateVertexBuffer(vertices) ||
        !CreateIndexBuffer(indices))
    {
        LOG_WARNING("Failed to load vertex- or indexbuffer...");
        success = false;
    }
    else
        std::cout << "Mesh: '" << filepath << "' created" << std::endl;

    //Cleaning
    vertices.clear();
    indices.clear();
    importer.FreeScene();

    return success;
}
