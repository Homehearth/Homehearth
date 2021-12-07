#pragma once
struct aiMesh;

class Mesh
{
private:
	ComPtr<ID3D11Buffer>		m_vertexBuffer;
	UINT						m_vertexCount;
	ComPtr<ID3D11Buffer>		m_indexBuffer;
	UINT						m_indexCount;
	std::shared_ptr<RMaterial>	m_material;
	bool						m_hasBones;

private:
	bool CreateVertexBuffer(const std::vector<simple_vertex_t>& vertices);
	bool CreateVertexBuffer(const std::vector<anim_vertex_t>& vertices);
	bool CreateIndexBuffer(const std::vector<UINT>& indices);
	bool LoadVertexSkinning(const aiMesh* aimesh, std::vector<anim_vertex_t>& vertices,
							const std::unordered_map<std::string, UINT>& boneMap);

public:
	Mesh();
	~Mesh();

	//Can create a combinded mesh with multiple aimeshes.
	//Bonemap is when loading in multiple meshes that can have same bones in it.
	bool Create(aiMesh* aimesh, const std::unordered_map<std::string, UINT>& boneMap);
	bool Create(const std::vector<aiMesh*>& aimeshes, const std::unordered_map<std::string, UINT>& boneMap);

	bool HasBones() const;
	const std::vector<sm::Vector2> GetTextureCoords() const;
	const std::shared_ptr<RTexture> GetTexture(const ETextureType& type) const;
	const std::shared_ptr<RMaterial> GetMaterial() const;

	//Set from a assimp-material
	bool SetMaterial(aiMaterial* aimat);
	//Set from the resourcemanager
	bool SetMaterial(const std::string& resourcename);
	//Set the material from a part of a mtl-file
	bool SetMaterialFromMTL(std::string& mtlpart);

	void Render(ID3D11DeviceContext* context);

};