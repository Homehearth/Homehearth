#pragma once

constexpr int MAX_PLAYERS_PER_LOBBY = 4;
constexpr int MAX_HEALTH = 100;

struct MinMaxProj_t
{
	float minProj;
	float maxProj;
	int minInxed;
	int maxIndex;
};

enum class TypeLight : UINT
{
	DIRECTIONAL,
	POINT
};


struct Plane_t
{
	sm::Vector3 point, normal;
};

struct Ray_t
{
	sm::Vector3 rayPos, rayDir;
	bool Intersects(Plane_t plane, sm::Vector3& outIntersectPoint)
	{
		rayDir.Normalize(rayDir);
		float dotAngle = plane.normal.Dot(rayDir);
		if (std::abs(dotAngle) < 0.001f)
			return false;

		sm::Vector3 p = plane.point - rayPos;
		float t = p.Dot(plane.normal) / dotAngle;
		if (t < 0)
			return false;

		outIntersectPoint = rayPos + rayDir * t;
		return true;
	}
};

struct InputState
{
	int axisHorizontal : 2;
	int axisVertical : 2;
	bool leftMouse : 1;
	bool rightMouse : 1;

	Ray_t mouseRay;

	uint32_t tick;
};

enum class GameMsg : uint8_t
{
	Client_Accepted,

	Lobby_Create,
	Lobby_Join,
	Lobby_Leave,
	Lobby_Accepted,
	Lobby_AcceptedLeave,
	Lobby_Invalid,
	Lobby_Update,
	Lobby_PlayerLeft,
	Lobby_PlayerJoin,

	Server_AssignID,
	Server_GetPing,

	Game_PlayerReady,
	Game_Start,
	Game_Snapshot,
	Game_AddEntity,
	Game_RemoveEntity,
	Game_BackToLobby,
	Game_WaveTimer,

	Game_PlayerInput,
	Grid_PlaceDefence
};

/*
	Basic objects like static meshes
*/
ALIGN16
struct simple_vertex_t
{
	sm::Vector3 position = {};
	sm::Vector2 uv		 = {};
	sm::Vector3 normal	 = {};
	sm::Vector3 tangent  = {};
	sm::Vector3 bitanget = {};
};

/*
	Debugging with collision boxes
*/
struct debug_vertex_t
{
	sm::Vector3 position = {};
	//sm::Vector3 color = {}; could be implemented if needed
};

/*
	Skeletal animated meshes
	Uses 3 bones per vertex which is cheaper than 4 
	without much of a noticeable difference
*/
ALIGN16
struct anim_vertex_t
{
	simple_vertex_t vertex	   = {};
	dx::XMUINT3		boneID	   = {};
	sm::Vector3		boneWeight = {};
};

ALIGN16
struct basic_model_matrix_t
{
	sm::Matrix worldMatrix;
};

ALIGN16
struct collider_hit_t
{
	int hit;
};

ALIGN16
struct camera_Matrix_t
{
	sm::Vector4 position;
	sm::Vector4 target;

	sm::Matrix projection;
	sm::Matrix view;
};

ALIGN16
struct light_t
{
	sm::Vector4 position	= {};	//Only in use on Point Lights
	sm::Vector4 direction	= {};	//Only in use on Directional Lights
	sm::Vector4 color		= {};	//Color and Intensity of the Lamp
	float		range		= 0;	//Only in use on Point Lights
	TypeLight	type		= TypeLight::DIRECTIONAL;	// 0 = Directional, 1 = Point
	UINT		enabled		= 0;	// 0 = Off, 1 = On
	float		padding		= 0;
};

static struct GridProperties_t
{
	sm::Vector3 position = sm::Vector3(0, 0, 0);
	sm::Vector2 mapSize = sm::Vector2(1200, 1200);
	std::string fileName = "GridMap.png";
	bool isVisible = true;

} Options;

enum class TileType
{
	DEFAULT,
	EMPTY,
	BUILDING,
	UNPLACABLE,
	DEFENCE
};