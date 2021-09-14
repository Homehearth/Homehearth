#pragma once
#include "EnginePCH.h"
#include "RMesh.h"

RMesh::RMesh()
{
    m_meshType = EMeshType::staticMesh;
}

RMesh::~RMesh()
{
    m_materials.clear();
    m_meshes.clear();
}

void RMesh::AddTextures(material_t& mat, const aiMaterial* aiMat)
{
    //Link together our format with assimps format
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
            mat.textures[(uint16_t)type.first] = ResourceManager::GetResource<RTexture>(filename);
        }
    }
    textureTypeMap.clear();
}

void RMesh::Render()
{
	//uint16_t currentMat = -1;
    UINT stride = sizeof(simple_vertex_t);
    UINT offset = 0;

	//For every submesh if it exists
	for (size_t m = 0; m < m_meshes.size(); m++)
	{
        //TODO: Fix rendering with materials
		//Switch material if needed
        /*
            if (currentMat != m_meshes[m].materialID)
		    {
			    currentMat = m_meshes[m].materialID;
                
                Set the materialbuffer
                D3D11Core::Get().DeviceContext()->

                m_materials[currentMat].ambient   //Go get the vector3
                m_materials[currentMat].textures[ETextureType::diffuse] to get the pointer to the texture
            }
        */
	
		//Draw with indexbuffer
        D3D11Core::Get().DeviceContext()->IASetVertexBuffers(0, 1, m_meshes[m].vertexBuffer.GetAddressOf(), &stride, &offset);
        D3D11Core::Get().DeviceContext()->IASetIndexBuffer(m_meshes[m].indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        D3D11Core::Get().DeviceContext()->DrawIndexed(m_meshes[m].indexCount, 0, 0);
	}
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
        std::cout << "Assimp error: " << importer.GetErrorString() << std::endl;
        importer.FreeScene();
        return false;
    }

    //Will not go on if the scene has no meshes
    if (!scene->HasMeshes())
    {
        importer.FreeScene();
        std::cout << "The scene has no meshes..." << std::endl;
        return false;
    }

    //Load in the materials
    for (unsigned int m = 0; m < scene->mNumMaterials; m++)
    {
        material_t mat;
        aiMaterial* aiMat = scene->mMaterials[m];

        //Basic float3-values
        aiMat->Get(AI_MATKEY_COLOR_AMBIENT, mat.ambient);
        aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, mat.diffuse);
        aiMat->Get(AI_MATKEY_COLOR_SPECULAR, mat.specular);
        aiMat->Get(AI_MATKEY_SHININESS, mat.shiniess);

        //Check what types of textures that exist and add them to a map
        AddTextures(mat, aiMat);

        m_materials.push_back(mat);
    }

    //For every mesh
    for (unsigned int m = 0; m < scene->mNumMeshes; m++)
    {
        const aiMesh* aimesh = scene->mMeshes[m];
        mesh_t submesh;
        submesh.materialID = aimesh->mMaterialIndex;

        std::vector<simple_vertex_t> vertices;
        std::vector<size_t> indices;
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
            vert.position = { aimesh->mVertices[v].x, aimesh->mVertices[v].y, aimesh->mVertices[v].z };
            vert.uv = { aimesh->mTextureCoords[0][v].x, aimesh->mTextureCoords[0][v].y };
            vert.normal = { aimesh->mNormals[v].x, aimesh->mNormals[v].y, aimesh->mNormals[v].z };
            vert.tangent = { aimesh->mTangents[v].x, aimesh->mTangents[v].y, aimesh->mTangents[v].z };
            vert.bitanget = { aimesh->mBitangents[v].x, aimesh->mBitangents[v].y, aimesh->mBitangents[v].z };
            vertices.push_back(vert);
        }


        //Indices
        for (unsigned int f = 0; f < aimesh->mNumFaces; f++)
        {
            const aiFace face = aimesh->mFaces[f];
            for (unsigned int id = 0; id < 3; id++)
                indices.push_back(face.mIndices[id]);
        }

        //Create vertex buffer
        Buffers::VertexBuffer vBuff;
        if (!vBuff.create(&vertices[0], vertices.size()))
        {
            std::cout << "Failed to create vertexbuffer..." << std::endl;
            importer.FreeScene();
            return false;
        }
        submesh.vertexBuffer = vBuff.Get();

        //Create index buffer
        Buffers::IndexBuffer iBuff;
        if (!iBuff.create(&indices[0], indices.size()))
        {
            std::cout << "Failed to create indexbuffer..." << std::endl;
            importer.FreeScene();
            return false;
        }
        submesh.indexBuffer = iBuff.Get();
        submesh.indexCount = uint32_t(iBuff.getIndexCount());

        //Cleaning
        vertices.clear();
        indices.clear();

        m_meshes.push_back(submesh);
    }

    importer.FreeScene();
    std::cout << "Mesh: '" << filepath << "' created" << std::endl;
    return true;
}
