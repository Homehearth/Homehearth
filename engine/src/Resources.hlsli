#ifndef _COMMON_HLSLI_
	#error You may not include this header directly.
#endif
static const float PI = 3.14159265359;

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
static const float3x3 ACESInputMat =
{
    { 0.59719, 0.35458, 0.04823 },
    { 0.07600, 0.90834, 0.01566 },
    { 0.02840, 0.13383, 0.83777 }
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const float3x3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367 },
    { -0.10208, 1.10813, -0.00605 },
    { -0.00327, -0.07276, 1.07602 }
};

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
    float lifeTime;
    float particleSizeMulitplier;
    float c_particleSpeed;
    
    float3 c_pPadding;
}

cbuffer ParticleMode : register(b9)
{
    uint c_pParticleType;
    float3 c_pPadding2;
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

cbuffer BlurSettings : register(b11)
{
    uint c_blurRadius;
    bool c_useVertical;
    uint c_blurType;
    float padding;
    float4 c_weights[MAXWEIGHTS / 4];
}
cbuffer TextureEffectCB : register(b7)
{
    uint c_frequency;
    uint c_amplitude;
    float c_counter;
    float c_padding;
};

cbuffer DoFSettings : register(b12)
{
    float4x4    c_inverseView;
    float4x4    c_inverseProjection;
    uint        c_dofType;
    float3      dofPadding;
    //float4      c_playerPos;
}
cbuffer SkyboxTint : register(b13)
{
    float3      c_tint = 1.f;
    float       pad;
}



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
Texture2D t_decal                   : register(t12);
Texture2DArray t_shadowMaps         : register(t13);

// StructuredBuffers.
StructuredBuffer<float4x4> sb_boneTransforms        : register(t9); // read as column major, actually is row major.
StructuredBuffer<Light> sb_lights                   : register(t10);
StructuredBuffer<float4x4> sb_decaldata             : register(t16);
StructuredBuffer<VertexParticleIn> particlesSRV     : register(t17);
StructuredBuffer<float> randomNumbers               : register(t18);

RWStructuredBuffer<VertexParticleIn> particlesUAV   : register(u7);


//Nikkis stuff:

Texture2D t_underWaterEdge     : register(t18);
Texture2D t_waterFloorTexture  : register(t19);
Texture2D t_waterTexture       : register(t20);
Texture2D t_waterTextureN      : register(t21);
Texture2D t_waterBlend         : register(t22);

RWTexture2D<unorm float4> u_waterFloorTexture : register(u5);
RWTexture2D<float4> u_waterTexture      : register(u6);
RWTexture2D<float4> u_waterTextureN     : register(u7);
//RWTexture2D<float4> u_waterBlend        : register(u5);
RWTexture2D<float4> u_underWaterEdge    : register(u9);

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

//Blur Pass
RWTexture2D<unorm float4> t_bufferRead      : register(u0);
RWTexture2D<unorm float4> t_bufferOut       : register(u1);
RWTexture2D<unorm float4> t_inFocus         : register(u2);
RWTexture2D<unorm float4> t_outOfFocus      : register(u3);
RWTexture2D<float4> t_dofOut                : register(u4);
RWTexture2D<float4> t_backBufferOut         : register(u5);

// Forward+
//RWStructuredBuffer<PointLight> rw_pointLights : register();
//RWStructuredBuffer<DirectionalLight> rw_directionalLights : register();
//RWStructuredBuffer<Frustum> rw_frustums : register();