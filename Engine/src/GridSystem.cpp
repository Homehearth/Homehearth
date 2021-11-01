#include "EnginePCH.h"
#include "GridSystem.h"
#define STB_IMAGE_IMPLEMENTATION

GridSystem::GridSystem()
{

}

GridSystem::~GridSystem()
{
}

void GridSystem::Initialize(sm::Vector2 mapSize, sm::Vector3 position, std::string fileName)
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

			rgba.x = (float)pixelValues.at(0 + ((row + (col * (int)m_gridSize.y)) * 4));
			rgba.y = (float)pixelValues.at(1 + ((row + (col * (int)m_gridSize.y)) * 4));
			rgba.z = (float)pixelValues.at(2 + ((row + (col * (int)m_gridSize.y)) * 4));
			rgba.w = (float)pixelValues.at(3 + ((row + (col * (int)m_gridSize.y)) * 4));

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

			Tile tileTemp;
			tileTemp.Initialize({ (float)(m_mapSize.x / m_gridSize.x),(float)(m_mapSize.y / m_gridSize.y) }, { (float)row, (float)col }, { 0.0f, 0.0f, 0.0f }, tileTypeTemp);
			m_tiles.push_back(tileTemp);

			float tileHalfWidth = tileTemp.GetHalfWidth();
			sm::Vector3 tilePosition = {m_position.x + (tileHalfWidth * row) + tileHalfWidth/2, m_position.y + 0.f , -(tileHalfWidth * col) - (tileHalfWidth/2) + m_position.z};
			m_tilePosiitons.push_back(tilePosition);
			tileTemp.SetPosition(tilePosition);
		}
	}

	stbi_image_free(pixelsData);
}

std::vector<sm::Vector3>* GridSystem::GetTilePositions()
{
	return &m_tilePosiitons;
}

std::vector<Tile>* GridSystem::GetTiles()
{
	return &m_tiles;
}