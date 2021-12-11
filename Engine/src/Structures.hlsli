#ifndef _COMMON_HLSLI_
	#error You may not include this header directly.
#endif

struct VertexIn
{
    float3 pos              : POSITION;
    float2 uv               : TEXCOORD;
    float3 normal           : NORMAL;
    float3 tangent          : TANGENT;
    float3 biTangent        : BINORMAL;
};

struct VertexOut
{
    float4 pos              : SV_POSITION;
    float2 uv               : TEXCOORD;
    float3 normal           : NORMAL;
    float3 tangent          : TANGENT;
    float3 biTangent        : BINORMAL;
    float4 worldPos         : WORLDPOSITION;
};

struct VertexBoneIn
{
    float3 pos              : POSITION;
    float2 uv               : TEXCOORD;
    float3 normal           : NORMAL;
    float3 tangent          : TANGENT;
    float3 biTangent        : BINORMAL;
    uint4  boneIDs          : BONEIDS;
    float4 boneWeights      : BONEWEIGHTS;
};

struct VertexOutSB
{
    float3 texCoord : SKYTEXCOORD;
    float4 posCS    : SV_Position;
};

struct PixelIn
{
    float4 pos              : SV_POSITION;
    float2 uv               : TEXCOORD;
    float3 normal           : NORMAL;
    float3 tangent          : TANGENT;
    float3 biTangent        : BINORMAL;
    float4 worldPos         : WORLDPOSITION;
};

struct PixelInSB
{
    float3 texCoord : SKYTEXCOORD;
    float4 posCS    : SV_Position;
};

struct PixelOut
{
    float4 color        : SV_Target0;
    float4 brightColor  : SV_Target1;
};

struct ComputeShaderIn
{
    uint3 groupID           : SV_GroupID;          
    uint3 groupThreadID     : SV_GroupThreadID;    
    uint3 dispatchThreadID  : SV_DispatchThreadID; 
    uint  groupIndex        : SV_GroupIndex;       
};

struct VertexParticleIn
{
    float4  pos             : POSITION;
    float4  velocity        : VELOCITY;
    float4  color           : COLOR;
    float2  size            : SIZE;
    uint    type            : TYPE;
    float   life            : LIFE;
};

struct VertexParticleOut
{
    float4 pos              : SV_POSITION;
    float4 color            : COLOR;
    float2 size             : SIZE;
};

struct PixelParticleIn
{
    float4 pos              : SV_POSITION;
    float4 color            : COLOR;
    float3 normal           : NORMAL;
    float2 uv               : TEXCOORD;
    float4 worldPos         : WORLDPOS;
};

struct Light
{
    float4      position;    //Only in use on Point Lights
    float4      direction;   //Only in use on Directional Lights
    float4      color;       //Color and Intensity of the Lamp
    float       range;       //Only in use on Point Lights
    int         type;        // 0 = Directional, 1 = Point
    uint        enabled;     // 0 = Off, 1 = On
    float       intensity;
	float4x4    lightMatrix; // Matrix to transform world point to light clip-space
    int         shadowIndex; // index in to the shadowmap array
    float3      padding;
};

struct Plane
{
    float3 normal;
    float  distanceToOrigin;
};

struct Frustum
{
    Plane planes[4];   // left, right, top, bottom frustum planes.
};

struct Sphere
{
    float3 center;
    float  radius;
};
