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

cbuffer MatPropertiesCB : register(b2)
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

cbuffer LightsInfoCB : register(b3)
{
    float4 c_info = float4(0.f, 0.f, 0.f, 0.f);
}

cbuffer ParticleUpdate : register(b4)
{
    float4 emitterPosition;
    float deltaTime;
    uint counter;
    float lifeTime;
    float particleSizeMulitplier;
    float c_particleSpeed;
    float3 c_pPadding;
}

cbuffer ParticleMode : register(b5)
{
    uint c_pParticleType;
    float3 c_pPadding2;
}

cbuffer DecalInfoCB : register(b6)
{
    float4 infoData = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4x4 decal_projection;
}

cbuffer BlurSettings : register(b7)
{
    uint c_blurRadius;
    bool c_useVertical;
    uint c_blurType;
    float padding;
    float4 c_weights[MAXWEIGHTS / 4];
}

cbuffer TextureEffectCB : register(b8)
{
    uint c_frequency;
    uint c_amplitude;
    float c_counter;
    float c_padding;
};

cbuffer DoFSettings : register(b9)
{
    float4x4    c_inverseProjection;
    uint        c_dofType;
    float3      dofPadding;
}

cbuffer SkyboxTint : register(b10)
{
    float3      c_tint = 1.f;
    float       pad;
}

cbuffer ScreenToViewParamsCB : register(b11) 
{
    float4x4 inverseProjection;
    float2 screenDimensions;
}

cbuffer DispatchParamsCB : register (b12) 
{
    uint4 numThreadGroups;
    uint4 numThreads;
}


//---------------------------------------------------------------------------
//	Samplers.
//---------------------------------------------------------------------------

SamplerState s_point		: register(s0);
SamplerState s_linear		: register(s1);
SamplerState s_anisotropic	: register(s2);
SamplerState s_cubeSamp     : register(s3);
SamplerState s_linearClamp  : register(s4);

//---------------------------------------------------------------------------
//	SRV's.
//---------------------------------------------------------------------------

// Textures
Texture2D t_depth					                : register(t0);
Texture2D t_albedo					                : register(t1);
Texture2D t_normal					                : register(t2);
Texture2D t_metalness				                : register(t3);
Texture2D t_roughness				                : register(t4);
Texture2D t_aomap					                : register(t5);
Texture2D t_displace				                : register(t6);
Texture2D t_opacitymask				                : register(t7);
Texture2D<uint2> t_pointLightGrid	                : register(t8);

// StructuredBuffers.
StructuredBuffer<float4x4> sb_boneTransforms        : register(t9); // read as column major, actually is row major.
StructuredBuffer<Light> sb_lights                   : register(t10);
StructuredBuffer<float> randomNumbers               : register(t11);
Texture2D t_decal                                   : register(t12);
Texture2DArray t_shadowMaps                         : register(t13);
StructuredBuffer<float4x4> sb_decaldata             : register(t16);
StructuredBuffer<VertexParticleIn> particlesSRV     : register(t17);

//Nikkis stuff:
Texture2D t_underWaterEdge                          : register(t18);
Texture2D t_waterFloorTexture                       : register(t19);
Texture2D t_waterTexture                            : register(t20);
Texture2D t_waterTextureN                           : register(t21);
Texture2D t_waterBlend                              : register(t22);

Texture2D<uint2> t_lightGrid                        : register(t23);
StructuredBuffer<Frustum> sb_frustums_in            : register(t24);    // Precomputed frustums for the grid.
StructuredBuffer<uint> sb_lightIndexList            : register(t25);
Texture2D t_lightCountHeatMap                       : register(t26);

TextureCube t_radiance                              : register(t96);
TextureCube t_irradiance                            : register(t97);
TextureCube t_sky                                   : register(t98);
Texture2D t_BRDFLUT                                 : register(t99);

//---------------------------------------------------------------------------
//	UAV's.
//---------------------------------------------------------------------------

// RWStructuredBuffers.

// ForwardPlus (0)
RWStructuredBuffer<Frustum> rw_frustums_out             : register(u0);

// Global counter for current index into the light index list.
RWStructuredBuffer<uint> rw_opaq_LightIndexCounter      : register(u1);
RWStructuredBuffer<uint> rw_trans_LightIndexCounter     : register(u2);

// Light index lists and light grids.
RWStructuredBuffer<uint> rw_opaq_LightIndexList         : register(u3);
RWStructuredBuffer<uint> rw_trans_LightIndexList        : register(u4);
      
// Each texel is a 2-component unsigned integer vector.
// The light grid texture is created using the R32G32_UINT format.
RWTexture2D<uint2> rw_opaq_LightGrid                    : register(u5);
RWTexture2D<uint2> rw_trans_LightGrid                   : register(u6);

RWTexture2D<float4> rw_heatMap                          : register(u7);

// Blur Pass (2) 
RWTexture2D<unorm float4> t_bufferRead                  : register(u0);
RWTexture2D<unorm float4> t_bufferOut                   : register(u1);
RWTexture2D<unorm float4> t_inFocus                     : register(u2);
RWTexture2D<unorm float4> t_outOfFocus                  : register(u3);
RWTexture2D<float4> t_dofOut                            : register(u4);

// Water (1)
RWTexture2D<unorm float4> u_waterFloorTexture           : register(u5);
RWTexture2D<float4> u_waterTexture                      : register(u6);
RWTexture2D<float4> u_waterTextureN                     : register(u7);

// Particles (3)
RWStructuredBuffer<VertexParticleIn> particlesUAV       : register(u7);

