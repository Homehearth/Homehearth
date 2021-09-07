#pragma once
#include "EnginePCH.h"
#include "Mesh.h"

Mesh::Mesh()
{
    m_meshType = EMeshType::staticMesh;
    m_isVisible = true;
}

Mesh::~Mesh()
{
    m_materials.clear();
    m_meshes.clear();
}

void Mesh::AddTextures(material_t& mat, const aiMaterial* aiMat)
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

            ADD_RESOURCE(RTexture, filename);
            RTexture* texture = GET_RESOURCE(RTexture, filename);
            if (texture)
                mat.textures[type.first] = texture;
        }
    }
    textureTypeMap.clear();
}

bool Mesh::Initialize(const std::string& filepath)
{
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
        const aiMesh* mesh = scene->mMeshes[m];
        submesh_t submesh;
        submesh.materialID = mesh->mMaterialIndex;

        std::vector<simple_vertex_t> vertices;
        std::vector<size_t> indices;
        vertices.reserve(mesh->mNumVertices);
        indices.reserve(size_t(mesh->mNumFaces) * 3);

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
            for (unsigned int v = 0; v < mesh->mNumVertices; v++)
            {
                simple_vertex_t vert = {};
                vert.position = { mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z };
                vert.uv = { mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y };
                vert.normal = { mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z };
                vert.tangent = { mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z };
                vert.bitanget = { mesh->mBitangents[v].x, mesh->mBitangents[v].y, mesh->mBitangents[v].z };
                vertices.push_back(vert);
            }
        
		
        //Indices
		for (unsigned int f = 0; f < mesh->mNumFaces; f++)
		{
			const aiFace face = mesh->mFaces[f];
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
		submesh.vertexBuffer = vBuff.Get();                 //GetAddressOf()?***

		//Create index buffer
		Buffers::IndexBuffer iBuff;
		if (!iBuff.create(&indices[0], indices.size()))
		{
            std::cout << "Failed to create indexbuffer..." << std::endl;
			importer.FreeScene();
			return false;
		}
		submesh.indexBuffer = iBuff.Get();                  //GetAddressOf?***
		submesh.indexCount = iBuff.getIndexCount();

        //Cleaning
        vertices.clear();
        indices.clear();

        m_meshes.push_back(submesh);
    }

    importer.FreeScene();
    std::cout << "Mesh: '" << filepath << "' created" << std::endl;
    return true;
}


void Mesh::Render()
{
    if (m_isVisible)
    {
        unsigned int currentMat = -1;

        //For every submesh if it exists
        for (size_t m = 0; m < m_meshes.size(); m++)
        {
            //Switch material and upload to GPU?
            if (currentMat != m_meshes[m].materialID)
            {
                currentMat = m_meshes[m].materialID;

                //m_materials[currentMat].ambient   //Go get the vector3
                //m_materials[currentMat].textures[ETextureType::diffuse] to get the pointer to the texture

            }
            
            //Draw everything with indexbuffer
            //m_meshes[m].vertexBuffer
            //m_meshes[m].indexBuffer
            //device.Draw()???
        }
    }
}
