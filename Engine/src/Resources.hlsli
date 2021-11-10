#ifndef _COMMON_HLSLI_
	#error You may not include this header directly.
#endif

//---------------------------------------------------------------------------
//	Constant buffers.
//---------------------------------------------------------------------------

cbuffer MatricesCB : register(b0)
{
    float4x4 c_world; //row major
}

cbuffer CameraCB : register(b1)
{
    float4   c_cameraPosition;
    float4   c_cameraTarget;
    float4x4 c_projection;  //row major
    float4x4 c_view;        //row major
}

cbuffer MatConstantsCB : register(b2)
{
    float3  c_ambient;
    float   c_shiniess;
    float3  c_diffuse;
    float   c_opacity;
    float3  c_specular;
}

cbuffer MatPropertiesCB : register(b3)
{
    //If a texture is set this will be 1
    int c_hasAlbedo;
    int c_hasNormal;
    int c_hasMetalness;
    int c_hasRoughness;
    int c_hasAoMap;
    int c_hasDisplace;
    int c_hasOpacity;
}

cbuffer LightsInfoCB : register(b4)
{
    float4 c_info = float4(0.f, 0.f, 0.f, 0.f);
}

cbuffer IsCollidingCB : register(b5)
{
    int c_colliding;
}

cbuffer ScreenToViewParamsCB : register(b6)
{
    float4x4 c_inverseProjection;
    float2 c_screenDimensions;
}

cbuffer DispatchParamsCB : register (b7)
{
    uint3 numThreadGroups;
    uint3 numThreads;
}

//---------------------------------------------------------------------------
//	Samplers.
//---------------------------------------------------------------------------

SamplerState s_point		: register(s0);
SamplerState s_linear		: register(s1);
SamplerState s_anisotropic	: register(s2);


//---------------------------------------------------------------------------
//	SRV's.
//---------------------------------------------------------------------------

// Textures
Texture2D t_depth					: register(t0);
Texture2D t_albedo					: register(t1);
Texture2D t_normal					: register(t2);
Texture2D t_metalness				: register(t3);
Texture2D t_roughness				: register(t4);
Texture2D t_aomap					: register(t5);
Texture2D t_displace				: register(t6);
Texture2D t_opacitymask				: register(t7);
Texture2D<uint2> t_pointLightGrid	: register(t8);

// StructuredBuffers.
StructuredBuffer<float4x4> sb_boneTransforms : register(t9); // read as column major, actually is row major.
StructuredBuffer<Light> sb_lights : register(t10);

// Forward+
//StructuredBuffer<PointLight> sb_pointLights : register();
//StructuredBuffer<DirectionalLight> sb_directionalLights : register();
//StructuredBuffer<uint> sb_pointLightIndexList : register();
StructuredBuffer<Frustum> sb_frustums : register(t11); // Precomputed frustums for the grid.


//---------------------------------------------------------------------------
//	UAV's.
//---------------------------------------------------------------------------

// RWStructuredBuffers.
RWStructuredBuffer<uint> opaq_lightIndexCounter     : register(u0);
RWStructuredBuffer<uint> trans_lightIndexCounter    : register(u1);
RWStructuredBuffer<uint> opaq_lightIndexList        : register(u2);
RWStructuredBuffer<uint> trans_lightIndexList       : register(u3);

// RWTexture2D.
RWTexture2D<uint2> opaq_lightGrid                   : register(u4);
RWTexture2D<uint2> trans_lightGrid                  : register(u5);

