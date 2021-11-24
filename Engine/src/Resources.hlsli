#ifndef _COMMON_HLSLI_
	#error You may not include this header directly.
#endif

static const float PI = 3.14159265359;
#define MAXWEIGHTS 8

#include "Structures.hlsli"
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

cbuffer ParticleUpdate : register(b8)
{
    float4 emitterPosition;
    float deltaTime;
    uint counter;
}

cbuffer DecalInfoCB : register(b10)
{
    float4 infoData = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4x4 decal_projection;
}

cbuffer BlurSettings : register(b11)
{
    uint c_blurRadius;
    bool c_useVertical;
    uint c_blurType;
    float padding;
    float4 c_weights[MAXWEIGHTS / 4];
}

cbuffer InverseMatrices : register(b12)
{
    float4x4    c_inverseView;
    float4x4    c_inverseProjection;
    uint        c_dofType;
    float3      dofPadding;
}



//---------------------------------------------------------------------------
//	Samplers.
//---------------------------------------------------------------------------

SamplerState s_point		: register(s0);
SamplerState s_linear		: register(s1);
SamplerState s_anisotropic	: register(s2);
SamplerState s_cubeSamp     : register(s3);


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
Texture2D t_decal                   : register(t12);
Texture2DArray t_shadowMaps         : register(t13);

// StructuredBuffers.
StructuredBuffer<float4x4> sb_boneTransforms        : register(t9); // read as column major, actually is row major.
StructuredBuffer<Light> sb_lights                   : register(t10);
StructuredBuffer<float4x4> sb_decaldata             : register(t16);
StructuredBuffer<VertexParticleIn> particlesSRV     : register(t17);
StructuredBuffer<float> randomNumbers               : register(t18);

RWStructuredBuffer<VertexParticleIn> particlesUAV   : register(u7);


// Forward+
//StructuredBuffer<PointLight> sb_pointLights : register();
//StructuredBuffer<DirectionalLight> sb_directionalLights : register();
//StructuredBuffer<uint> sb_pointLightIndexList : register();
//StructuredBuffer<Frustum> sb_frustums : register();

TextureCube t_radiance              : register(t96);
TextureCube t_irradiance            : register(t97);
TextureCube t_sky                   : register(t98);
Texture2D t_BRDFLUT                 : register(t99);


//---------------------------------------------------------------------------
//	UAV's.
//---------------------------------------------------------------------------

// RWStructuredBuffers.

//Blur Pass
RWTexture2D<unorm float4> t_bufferRead      : register(u0);
RWTexture2D<unorm float4> t_bufferOut       : register(u1);
RWTexture2D<unorm float4> t_inFocus         : register(u2);
RWTexture2D<unorm float4> t_outOfFocus      : register(u3);
RWTexture2D<float4> t_dofOut                : register(u4);

// Forward+
//RWStructuredBuffer<PointLight> rw_pointLights : register();
//RWStructuredBuffer<DirectionalLight> rw_directionalLights : register();
//RWStructuredBuffer<Frustum> rw_frustums : register();