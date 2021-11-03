#include "EnginePCH.h"
#include "GridSystem.h"
#define STB_IMAGE_IMPLEMENTATION

GridSystem::GridSystem()
{

}

GridSystem::~GridSystem()
{
}

void GridSystem::Initialize(sm::Vector2 mapSize, sm::Vector3 position, std::string fileName, HeadlessScene* scene)
{
	m_position = position;
	m_mapSize = mapSize;

	std::string m_filepath = TEXTUREPATH + fileName;
	int width = 0;
	int height = 0;
	int comp = 0;

	//std::shared_ptr<RTexture> texture = ResourceManager::Get().GetResource<RTexture>(fileName);
	//bool added = ResourceManager::Get().AddResource(fileName, texture);
	//std::shared_ptr<RTexture> texture = std::make_shared<RTexture>();
	//texture->GetImageData(fileName);
	//texture->GetSize();

	unsigned char* pixelsData = stbi_load(m_filepath.c_str(), &width, &height, &comp, STBI_rgb_alpha); //TODO: find out what to do with this

	std::vector<int> pixelValues;
	m_gridSize = { (float)width, (float)height };

	for (int i = 0; i < m_gridSize.x * m_gridSize.y * 4; i++)
	{
		pixelValues.push_back((int)pixelsData[i]);
	}

	//Initialize all tiles
	for (int col = 0; col < m_gridSize.y; col++)
	{
		for (int row = 0; row < m_gridSize.x; row++)
		{
			TileType tileTypeTemp = TileType::DEFAULT;

			sm::Vector4 rgba;

			rgba.x = (float)pixelValues.at(0 + ((row + (col * (size_t)m_gridSize.y)) * 4));
			rgba.y = (float)pixelValues.at(1 + ((row + (col * (size_t)m_gridSize.y)) * 4));
			rgba.z = (float)pixelValues.at(2 + ((row + (col * (size_t)m_gridSize.y)) * 4));
			rgba.w = (float)pixelValues.at(3 + ((row + (col * (size_t)m_gridSize.y)) * 4));

			//std::cout << "RGBA: " << rgba.x << " " << rgba.y << " " << rgba.z << " " << rgba.w << std::endl;

			if (rgba == sm::Vector4{ 100, 100, 100, 255 })
			{
				tileTypeTemp = TileType::EMPTY;
			}
			if (rgba == sm::Vector4{ 255, 51, 0, 255 })
			{
				tileTypeTemp = TileType::BUILDING;
			}
			if (rgba == sm::Vector4{ 255, 234, 0, 255 } || rgba == sm::Vector4{ 0, 21, 255, 255 })
			{
				tileTypeTemp = TileType::UNPLACABLE;
			}
			if (rgba == sm::Vector4{ 0, 0, 0, 255 })
			{
				tileTypeTemp = TileType::DEFAULT;
			}

			float tileHalfWidth = (float)(((m_mapSize.x / m_gridSize.x)) / 2);
			sm::Vector3 tilePosition = { m_position.x + (tileHalfWidth * row) + tileHalfWidth / 2, m_position.y + 0.f , -(tileHalfWidth * col) - (tileHalfWidth / 2) + m_position.z };
			m_tilePosiitons.push_back(tilePosition);

			Entity tileEntity = scene->CreateEntity();
			comp::Tile* tile = tileEntity.AddComponent<comp::Tile>();
			tile->gridID = { (float)row, (float)col };
			tile->halfWidth = tileHalfWidth;
			tile->type = tileTypeTemp;
			comp::Transform* transform = tileEntity.AddComponent<comp::Transform>();
			transform->position = tilePosition;
			transform->position.y = 0.5;

			transform->scale = { 4.2f, 0.5f, 4.2f };

			// Uncomment this if you want to send Tiles to client
			//tileEntity.AddComponent<comp::Network>();

			m_tiles.push_back(tileEntity);
		}
	}
	stbi_image_free(pixelsData);
}

void GridSystem::Initialize2(sm::Vector2 mapSize, sm::Vector3 position, std::string fileName, Scene* scene)
{
	m_position = position;
	m_mapSize = mapSize;

	std::string m_filepath = TEXTUREPATH + fileName;
	int width = 0;
	int height = 0;
	int comp = 0;

	unsigned char* pixelsData = stbi_load(m_filepath.c_str(), &width, &height, &comp, STBI_rgb_alpha); //TODO: find out what to do with this

	std::vector<int> pixelValues;
	m_gridSize = { (float)width, (float)height };

	for (int i = 0; i < m_gridSize.x * m_gridSize.y * 4; i++)
	{
		pixelValues.push_back((int)pixelsData[i]);
	}

	//Initialize all tiles
	for (int col = 0; col < m_gridSize.y; col++)
	{
		for (int row = 0; row < m_gridSize.x; row++)
		{
			TileType tileTypeTemp = TileType::DEFAULT;

			sm::Vector4 rgba;

			rgba.x = (float)pixelValues.at(0 + ((row + (col * (size_t)m_gridSize.y)) * 4));
			rgba.y = (float)pixelValues.at(1 + ((row + (col * (size_t)m_gridSize.y)) * 4));
			rgba.z = (float)pixelValues.at(2 + ((row + (col * (size_t)m_gridSize.y)) * 4));
			rgba.w = (float)pixelValues.at(3 + ((row + (col * (size_t)m_gridSize.y)) * 4));

			//std::cout << "RGBA: " << rgba.x << " " << rgba.y << " " << rgba.z << " " << rgba.w << std::endl;

			if (rgba == sm::Vector4{ 100, 100, 100, 255 })
			{
				tileTypeTemp = TileType::EMPTY;
			}
			if (rgba == sm::Vector4{ 255, 51, 0, 255 })
			{
				tileTypeTemp = TileType::BUILDING;
			}
			if (rgba == sm::Vector4{ 255, 234, 0, 255 } || rgba == sm::Vector4{ 0, 21, 255, 255 })
			{
				tileTypeTemp = TileType::UNPLACABLE;
			}
			if (rgba == sm::Vector4{ 0, 0, 0, 255 })
			{
				tileTypeTemp = TileType::DEFAULT;
			}

			float tileHalfWidth = (m_mapSize.x / m_gridSize.x) / 2.0f;
			sm::Vector3 tilePosition = { m_position.x + (tileHalfWidth * row) + tileHalfWidth / 2, m_position.y + 0.f , -(tileHalfWidth * col) - (tileHalfWidth / 2) + m_position.z };
			m_tilePosiitons.push_back(tilePosition);

			Entity tileEntity = scene->CreateEntity();
			comp::Tile* tile = tileEntity.AddComponent<comp::Tile>();
			tile->gridID = { (float)row, (float)col };
			tile->halfWidth = tileHalfWidth;
			tile->type = tileTypeTemp;
			comp::Transform* transform = tileEntity.AddComponent<comp::Transform>();
			transform->position = tilePosition;
			transform->scale = { 4.2f, 0.5f, 4.2f };

			m_tiles.push_back(tileEntity);
		}
	}

	stbi_image_free(pixelsData);
}
sm::Vector2 GridSystem::GetGridSize() const
{
	return m_gridSize;
}
std::vector<sm::Vector3>* GridSystem::GetTilePositions()
{
	return &m_tilePosiitons;
}
Entity* GridSystem::GetTileByID(sm::Vector2 id)
{
	if (id.x >= 0 && id.y >= 0 && id.x < m_gridSize.x && id.y < m_gridSize.y)
	{
		for (int i = 0; i < m_tiles.size(); i++)
		{
			comp::Tile* tile = m_tiles.at(i).GetComponent<comp::Tile>();
			if (tile->gridID == id);
			{
				return &m_tiles.at(i);
			}
		}
	}
	return nullptr;
}

std::vector<Entity>* GridSystem::GetTiles()
{
	return &m_tiles;
}
