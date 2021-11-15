#ifndef _COMMON_HLSLI_
	#error You may not include this header directly.
#endif
static const float PI = 3.14159265359;
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
cbuffer DecalInfoCB : register(b10)
{
    float4 infoData = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4x4 decal_projection;
}
cbuffer DeltaCB : register(b6)
{
    float c_deltaTime;
}

cbuffer TextureEffectCB : register(b7)
{
    float c_frequency;
    float c_amplitude;
};

//---------------------------------------------------------------------------
//	Samplers.
//---------------------------------------------------------------------------

SamplerState s_point		: register(s0);
SamplerState s_linear		: register(s1);
SamplerState s_anisotropic	: register(s2);
SamplerState s_cubeSamp : register(s3);


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
Texture2D t_decal : register(t12);
Texture2D t_decalAlpha : register(t13);

// StructuredBuffers.
StructuredBuffer<float4x4> sb_boneTransforms : register(t9); // read as column major, actually is row major.
StructuredBuffer<Light> sb_lights : register(t10);
StructuredBuffer<float4x4> sb_decaldata : register(t16);

//Nikkis stuff:
Texture2D t_waterBlend       : register(t17);
Texture2D t_underWaterEdge   : register(t18);
Texture2D t_underWaterFloor  : register(t19);
Texture2D t_waterTexture     : register(t20);
Texture2D t_waterTextureN    : register(t21);

// Forward+
//StructuredBuffer<PointLight> sb_pointLights : register();
//StructuredBuffer<DirectionalLight> sb_directionalLights : register();
//StructuredBuffer<uint> sb_pointLightIndexList : register();
//StructuredBuffer<Frustum> sb_frustums : register();

TextureCube t_radiance : register(t96);
TextureCube t_irradiance : register(t97);
TextureCube t_sky : register(t98);
Texture2D t_BRDFLUT : register(t99);

//---------------------------------------------------------------------------
//	UAV's.
//---------------------------------------------------------------------------

// RWStructuredBuffers.


// Forward+
//RWStructuredBuffer<PointLight> rw_pointLights : register();
//RWStructuredBuffer<DirectionalLight> rw_directionalLights : register();
//RWStructuredBuffer<Frustum> rw_frustums : register();