#include "EnginePCH.h"
#include "Mesh.h"

Mesh::Mesh()
{
	m_vertexCount	= 0;
	m_indexCount	= 0;
	m_hasBones		= false;
}

Mesh::~Mesh()
{
}

bool Mesh::CreateVertexBuffer(const std::vector<simple_vertex_t>& vertices)
{
    if (vertices.empty())
        return false;

    D3D11_BUFFER_DESC desc      = {};
    desc.ByteWidth              = static_cast<UINT>(sizeof(simple_vertex_t) * vertices.size());
    desc.Usage                  = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    desc.BindFlags              = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags         = 0;
    desc.MiscFlags              = 0;
    desc.StructureByteStride    = 0;

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem                = &vertices[0];
    data.SysMemPitch            = 0;
    data.SysMemSlicePitch       = 0;

    m_vertexCount = static_cast<UINT>(vertices.size());
    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&desc, &data, m_vertexBuffer.GetAddressOf());
    return !FAILED(hr);
}

bool Mesh::CreateVertexBuffer(const std::vector<anim_vertex_t>& vertices)
{
    if (vertices.empty())
        return false;

    D3D11_BUFFER_DESC desc      = {};
    desc.ByteWidth              = static_cast<UINT>(sizeof(anim_vertex_t) * vertices.size());
    desc.Usage                  = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    desc.BindFlags              = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags         = 0;
    desc.MiscFlags              = 0;
    desc.StructureByteStride    = 0;

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem                = &vertices[0];
    data.SysMemPitch            = 0;
    data.SysMemSlicePitch       = 0;

    m_vertexCount = static_cast<UINT>(vertices.size());
    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&desc, &data, m_vertexBuffer.GetAddressOf());
    return !FAILED(hr);
}

bool Mesh::CreateIndexBuffer(const std::vector<UINT>& indices)
{
    if (indices.empty())
        return false;

    D3D11_BUFFER_DESC desc      = {};
    desc.ByteWidth              = static_cast<UINT>(sizeof(UINT) * indices.size());
    desc.Usage                  = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    desc.BindFlags              = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
    desc.CPUAccessFlags         = 0;
    desc.MiscFlags              = 0;

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem                = &indices[0];
    data.SysMemPitch            = 0;
    data.SysMemSlicePitch       = 0;

    m_indexCount = static_cast<UINT>(indices.size());
    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&desc, &data, m_indexBuffer.GetAddressOf());
    return !FAILED(hr);
}

bool Mesh::LoadVertexSkinning(const aiMesh* aimesh, std::vector<anim_vertex_t>& vertices, const std::unordered_map<std::string, UINT>& boneMap)
{
    //Data that will be used temporarily for this.
    std::vector<UINT> boneCounter;
    boneCounter.resize(vertices.size(), 0);

    //Go through all the bones
    for (UINT b = 0; b < aimesh->mNumBones; b++)
    {
        aiBone* aibone = aimesh->mBones[b];
        UINT boneNr = boneMap.at(aibone->mName.C_Str());

        //Go through all the vertices that the bone affect
        for (UINT v = 0; v < aibone->mNumWeights; v++)
        {
            UINT id = aibone->mWeights[v].mVertexId;
            float weight = aibone->mWeights[v].mWeight;

            switch (boneCounter[id]++)
            {
            case 0:
                vertices[id].boneIDs.x = boneNr;
                vertices[id].boneWeights.x = weight;
                break;
            case 1:
                vertices[id].boneIDs.y = boneNr;
                vertices[id].boneWeights.y = weight;
                break;
            case 2:
                vertices[id].boneIDs.z = boneNr;
                vertices[id].boneWeights.z = weight;
                break;
            case 3:
                vertices[id].boneIDs.w = boneNr;
                vertices[id].boneWeights.w = weight;
                break;
            default:
                //Vertex already has 4 bones connected
                break;
            };
        }
    }

    //Normalize the weights - have to sum up to 1.0
    for (size_t i = 0; i < vertices.size(); i++)
    {
        sm::Vector4 weights = vertices[i].boneWeights;
        //Total weight
        float total = weights.x + weights.y + weights.z + weights.w;
        //Avoid devide by 0
        if (total > 0.0f)
        {
            vertices[i].boneWeights = { weights.x / total,
                                        weights.y / total,
                                        weights.z / total,
                                        weights.w / total };
        }
    }

    //Freeing up space
    boneCounter.clear();
    return true;
}

bool Mesh::Create(aiMesh* aimesh, const std::unordered_map<std::string, UINT>& boneMap)
{
    std::vector<simple_vertex_t> simpleVertices;
    std::vector<anim_vertex_t>   skeletonVertices;
    std::vector<UINT> indices;
    //Offset for indices when moving between different submeshes
    UINT indexOffset = 0;

	//Load in vertices
	for (UINT v = 0; v < aimesh->mNumVertices; v++)
	{
		simple_vertex_t vert = {};
		vert.position       = { aimesh->mVertices[v].x,         aimesh->mVertices[v].y,       aimesh->mVertices[v].z };
		vert.uv             = { aimesh->mTextureCoords[0][v].x, aimesh->mTextureCoords[0][v].y };
		vert.normal         = { aimesh->mNormals[v].x,          aimesh->mNormals[v].y,        aimesh->mNormals[v].z };
		vert.tangent        = { aimesh->mTangents[v].x,         aimesh->mTangents[v].y,       aimesh->mTangents[v].z };
		vert.bitanget       = { aimesh->mBitangents[v].x,       aimesh->mBitangents[v].y,     aimesh->mBitangents[v].z };
		simpleVertices.push_back(vert);

		if (aimesh->HasBones())
		{
			anim_vertex_t animVert = {};
			animVert.position = vert.position;
			animVert.uv = vert.uv;
			animVert.normal = vert.normal;
			animVert.tangent = vert.tangent;
			animVert.bitanget = vert.bitanget;
			skeletonVertices.push_back(animVert);
			m_hasBones = true;
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
	indexOffset += (localMaxIndex + 1);

	//Load in the skeleton if it exist
	if (aimesh->HasBones())
	{
		skeletonVertices.shrink_to_fit();
		LoadVertexSkinning(aimesh, skeletonVertices, boneMap);
	}
    

    /*
        Creating buffers
    */
    indices.shrink_to_fit();
    if (!CreateIndexBuffer(indices))
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
    if (m_hasBones)
    {
        //Finally create the buffer
        skeletonVertices.shrink_to_fit();
        if (!CreateVertexBuffer(skeletonVertices))
        {
#ifdef _DEBUG
            LOG_ERROR("Failed to create vertexbuffer for skeleton...");
#endif
        }
    }
    else
    {
        simpleVertices.shrink_to_fit();
        if (!CreateVertexBuffer(simpleVertices))
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

bool Mesh::Create(const std::vector<aiMesh*>& aimeshes, const std::unordered_map<std::string, UINT>& boneMap)
{
    std::vector<simple_vertex_t> simpleVertices;
    std::vector<anim_vertex_t>   skeletonVertices;
    std::vector<UINT> indices;
    //Offset for indices when moving between different submeshes
    UINT indexOffset = 0;

    //Go through all the meshes
    for (UINT m = 0; m < aimeshes.size(); m++)
    {
        const aiMesh* aimesh = aimeshes[m];

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
                animVert.position = vert.position;
                animVert.uv = vert.uv;
                animVert.normal = vert.normal;
                animVert.tangent = vert.tangent;
                animVert.bitanget = vert.bitanget;
                skeletonVertices.push_back(animVert);
                m_hasBones = true;
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
        indexOffset += (localMaxIndex + 1);

        //Load in the skeleton if it exist
        if (aimesh->HasBones())
        {
            skeletonVertices.shrink_to_fit();
            LoadVertexSkinning(aimesh, skeletonVertices, boneMap);
        }
    }

    /*
        Creating buffers
    */
    indices.shrink_to_fit();
    if (!CreateIndexBuffer(indices))
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
    if (m_hasBones)
    {
        //Finally create the buffer
        skeletonVertices.shrink_to_fit();
        if (!CreateVertexBuffer(skeletonVertices))
        {
#ifdef _DEBUG
            LOG_ERROR("Failed to create vertexbuffer for skeleton...");
#endif
        }
    }
    else
    {
        simpleVertices.shrink_to_fit();
        if (!CreateVertexBuffer(simpleVertices))
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

bool Mesh::HasBones() const
{
    return m_hasBones;
}

const std::vector<sm::Vector2> Mesh::GetTextureCoords() const
{
    std::vector<sm::Vector2> texturecoords;

    ComPtr<ID3D11Buffer> tempResource;
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    UINT byteWidth = 0;

    if (m_hasBones)
        byteWidth = static_cast<UINT>(sizeof(anim_vertex_t) * m_vertexCount);
    else
        byteWidth = static_cast<UINT>(sizeof(simple_vertex_t) * m_vertexCount);

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
    D3D11Core::Get().DeviceContext()->CopyResource(tempResource.Get(), m_vertexBuffer.Get());

    std::vector<simple_vertex_t> simpleVertices;
    std::vector<anim_vertex_t>   animVertices;

    //Copy the data
    D3D11_MAPPED_SUBRESOURCE data;
    D3D11Core::Get().DeviceContext()->Map(tempResource.Get(), 0, D3D11_MAP_READ, 0, &data);

    if (m_hasBones)
    {
        animVertices.resize(m_vertexCount);
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
        simpleVertices.resize(m_vertexCount);
        memcpy(&simpleVertices[0], data.pData, byteWidth);

        //Go through the vector of vertices and get the texturecoords
        for (size_t v = 0; v < simpleVertices.size(); v++)
        {
            texturecoords.push_back(simpleVertices[v].uv);
        }
        simpleVertices.clear();
    }
    D3D11Core::Get().DeviceContext()->Unmap(tempResource.Get(), 0);

    return texturecoords;
}

const std::shared_ptr<RTexture> Mesh::GetTexture(const ETextureType& type) const
{
    if (m_material)
        return m_material->GetTexture(type);
    else
        return std::shared_ptr<RTexture>(nullptr);
}

const std::shared_ptr<RMaterial> Mesh::GetMaterial() const
{
    return m_material;
}

bool Mesh::SetMaterial(aiMaterial* aimat)
{
    bool materialSet = false;

    if (aimat)
    {
        aiString matName;
        aimat->Get(AI_MATKEY_NAME, matName);

        //Check if the material exists
        std::shared_ptr<RMaterial> material = ResourceManager::Get().GetResource<RMaterial>(matName.C_Str(), false);
        if (!material)
        {
            material = std::make_shared<RMaterial>();
            //Add the material to the resourcemanager if it was successfully created
            if (material->Create(aimat))
            {
                ResourceManager::Get().AddResource(matName.C_Str(), material);
                materialSet = true;
            }
            //Else - failed an therefore we shall not link it to the submesh
        }
        else
        {
            materialSet = true;
        }

        //Set the material
        if (materialSet)
            m_material = material;
    }

    return materialSet;
}

bool Mesh::SetMaterial(const std::string& resourcename)
{
    std::shared_ptr<RMaterial> material;
    material = ResourceManager::Get().GetResource<RMaterial>(resourcename);

    if (material)
    {
        m_material = material;
        return true;
    }
    else
    {
        return false;
    }
}

bool Mesh::SetMaterialFromMTL(std::string& mtlpart)
{
    bool success = false;
    std::shared_ptr<RMaterial> material = std::make_shared<RMaterial>();
    if (material->CreateFromMTL(mtlpart))
    {
        m_material = material;
        success = true;
    }
    return success;
}

void Mesh::Render(ID3D11DeviceContext* context)
{
    UINT offset = 0;
    UINT stride = 0;

	if (m_material)
        m_material->BindMaterial(context);

	if (m_hasBones)
		stride = sizeof(anim_vertex_t);
	else
		stride = sizeof(simple_vertex_t);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->DrawIndexed(m_indexCount, 0, 0);

	if (m_material)
        m_material->UnBindMaterial(context);
}
