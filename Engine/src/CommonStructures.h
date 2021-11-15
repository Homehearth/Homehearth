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

struct Vector2I
{
	int x = 0, y = 0;

	Vector2I(int&& x, int&& y) :x(x), y(y) {};
	Vector2I(int& x, int& y) :y(y), x(x) {};

	Vector2I() = default;

	bool operator==(const Vector2I& other)
	{
		return (x == other.x && y == other.y);
	}
	Vector2I& operator+(const Vector2I& other)
	{
		Vector2I toReturn(this->x + other.x, this->y + other.y);
		return toReturn;
	}
	Vector2I& operator+=(const Vector2I& other)
	{
		this->x += other.x;
		this->y += other.y;

		return *this;
	}
	Vector2I& operator-(const Vector2I& other)
	{
		Vector2I toReturn(this->x - other.x, this->y - other.y);
		return toReturn;
	}
	Vector2I& operator-=(const Vector2I& other)
	{
		this->x -= other.x;
		this->y -= other.y;

		return *this;
	}

};

struct Plane_t
{
	sm::Vector3 point, normal;
};

struct Ray_t
{
	sm::Vector3 origin, dir;
	bool Intersects(Plane_t plane, sm::Vector3* outIntersectPoint = nullptr)
	{
		dir.Normalize(dir);
		float dotAngle = plane.normal.Dot(dir);
		if (std::abs(dotAngle) < 0.001f)
			return false;

		sm::Vector3 p = plane.point - origin;
		float t = p.Dot(plane.normal) / dotAngle;
		if (t < 0)
			return false;

		if (outIntersectPoint)
		{
			*outIntersectPoint = origin + dir * t;
		}
		return true;
	}

	bool Intersects(const dx::BoundingOrientedBox& boxCollider)
	{

		/*
		 * computing all t-values for the ray
		 * and all planes belonging to the faces of the OBB.
		 * It returns the closest positive t-value
		 */

		float tmin = (std::numeric_limits<float>::min)();
		float tmax = (std::numeric_limits<float>::max)();

		sm::Vector3 p = boxCollider.Center - this->origin;

		std::array<sm::Vector3, 3> norms;
		norms[0] = sm::Vector3(1.0f, 0.0f, 0.0f);
		norms[1] = sm::Vector3(0.0f, 1.0f, 0.0f);
		norms[2] = sm::Vector3(0.0f, 0.0f, 1.0f);

		norms[0] = sm::Vector3::Transform(norms[0], boxCollider.Orientation);
		norms[1] = sm::Vector3::Transform(norms[1], boxCollider.Orientation);
		norms[2] = sm::Vector3::Transform(norms[2], boxCollider.Orientation);

		float halfSize[3];
		halfSize[0] = boxCollider.Extents.x;
		halfSize[1] = boxCollider.Extents.y;
		halfSize[2] = boxCollider.Extents.z;

		for (size_t i = 0; i < 3; i++)
		{
			const float e = norms[i].x * p.x + norms[i].y * p.y + norms[i].z * p.z;
			const float f = norms[i].x * this->dir.x + norms[i].y * this->dir.y + norms[i].z * this->dir.z;

			//CheckCollisions normal face is not ortogonal to ray direction
			if (abs(f) > 0.001f)
			{
				float t1 = (e + halfSize[i]) / f;
				float t2 = (e - halfSize[i]) / f;

				if (t1 > t2)
				{
					std::swap(t1, t2);
				}
				if (t1 > tmin)
				{
					tmin = t1;
				}
				if (t2 < tmax)
				{
					tmax = t2;
				}
				if (tmin > tmax)
				{
					return false;
				}
				//if true, then the box is behind the rayorigin.
				if (tmax < 0)
				{
					return false;
				}

			}
			/**executed if the ray is parallel to the slab
			 * (and so cannot intersect it); it tests if the ray is outside the slab.
			 * If so, then the ray misses the box and the test terminates.
			 */
			else if (-e - halfSize[i] > 0 || -e + halfSize[i] < 0)
			{
				return false;
			}

		}

		//if (tmin > 0)
		//	t = tmin;
		//else
		//	t = tmax;

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
	Game_UpdateComponent,
	Game_RemoveEntity,
	Game_BackToLobby,
	Game_WaveTimer,

	Game_PlayerAttack,
	Game_AddNPC,
	Game_RemoveNPC,
	Game_PlayerInput
};

/*
	Basic objects like static meshes
*/
ALIGN16
struct simple_vertex_t
{
	sm::Vector3 position = {};
	sm::Vector2 uv = {};
	sm::Vector3 normal = {};
	sm::Vector3 tangent = {};
	sm::Vector3 bitanget = {};
};

/*
	Skeletal animated model.
	Uses 4 bones per vertex for now.
	If optimization is needed, consider lowering to 3.
*/
ALIGN16
struct anim_vertex_t
{
	sm::Vector3 position = {};
	sm::Vector2	uv = {};
	sm::Vector3	normal = {};
	sm::Vector3	tangent = {};
	sm::Vector3	bitanget = {};
	dx::XMUINT4	boneIDs = {};
	sm::Vector4	boneWeights = {};
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
	sm::Vector4 position = {};	//Only in use on Point Lights
	sm::Vector4 direction = {};	//Only in use on Directional Lights
	sm::Vector4 color = {};	//Color and Intensity of the Lamp
	float		range = 0;	//Only in use on Point Lights
	TypeLight	type = TypeLight::DIRECTIONAL;	// 0 = Directional, 1 = Point
	UINT		enabled = 0;	// 0 = Off, 1 = On
	float		padding = 0;
};

static struct GridProperties_t
{
	sm::Vector3 position = sm::Vector3(0, 0, 0);
	Vector2I mapSize = Vector2I(600, 600);
	std::string fileName = "GridMap.png";
	bool isVisible = true;

} gridOptions;

struct Node
{
	float f = FLT_MAX, g = FLT_MAX, h = FLT_MAX;
	sm::Vector3 position;
	Vector2I id;
	std::vector<Node*> connections;
	Node* parent = nullptr;
	bool reachable = true;
	Node(Vector2I id) : id(id) {};

	void ResetFGH()
	{
		f = FLT_MAX, g = FLT_MAX, h = FLT_MAX;
	}
	std::vector<Node*> GetDiagonalConnections()
	{
		std::vector<Node*> DiagConnections;
		for (Node* connection : connections)
		{
			if (abs(connection->id.x - this->id.x) == 1 && abs(connection->id.y - this->id.y) == 1)
			{
				DiagConnections.push_back(connection);
			}
		}
		return DiagConnections;
	}
	bool ConnectionAlreadyExists(Node* other)
	{
		for (Node* node : connections)
		{
			if (node == other)
			{
				return true;
			}
		}
		return false;
	}
};
