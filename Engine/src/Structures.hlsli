#ifndef _COMMON_HLSLI_
	#error You may not include this header directly.
#endif

struct VertexIn
{
    float3 pos          : POSITION;
    float2 uv           : TEXCOORD;
    float3 normal       : NORMAL;
    float3 tangent      : TANGENT;
    float3 biTangent    : BINORMAL;
};

struct VertexBoneIn
{
    float3 pos          : POSITION;
    float2 uv           : TEXCOORD;
    float3 normal       : NORMAL;
    float3 tangent      : TANGENT;
    float3 biTangent    : BINORMAL;
    uint4  boneIDs      : BONEIDS;
    float4 boneWeights  : BONEWEIGHTS;
};

struct VertexOut
{
    float4 pos          : SV_POSITION;
    float2 uv           : TEXCOORD;
    float3 normal       : NORMAL;
    float3 tangent      : TANGENT;
    float3 biTangent    : BINORMAL;
    float4 worldPos     : WORLDPOSITION;
};

struct PixelIn
{
    float4 pos          : SV_POSITION;
    float2 uv           : TEXCOORD;
    float3 normal       : NORMAL;
    float3 tangent      : TANGENT;
    float3 biTangent    : BINORMAL;
    float4 worldPos     : WORLDPOSITION;
};

struct ComputeShaderIn
{
    uint3 GroupID           : SV_GroupID;          
    uint3 GroupThreadID     : SV_GroupThreadID;    
    uint3 DispatchThreadID  : SV_DispatchThreadID; 
    uint  GroupIndex        : SV_GroupIndex;       
};

struct Light
{
    float4 position;    //Only in use on Point Lights
    float4 direction;   //Only in use on Directional Lights
    float4 color;       //Color and Intensity of the Lamp
    float  range;       //Only in use on Point Lights
    int    type;        // 0 = Directional, 1 = Point
    uint   enabled;     // 0 = Off, 1 = On
    float  padding;
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