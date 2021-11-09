cbuffer Matrices : register(b0)
{
	float4x4 c_world; //row major
}

cbuffer Camera : register(b1)
{
    float4   c_cameraPosition;
    float4   c_cameraTarget;
    float4x4 c_projection;  //row major
    float4x4 c_view;        //row major
}

struct VertexIn
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biTangent : BINORMAL;
};

// Depth pre-pass.
float4 main(VertexIn input) : SV_POSITION
{
    float4 position = float4(input.pos, 1.0f);
    position = mul(c_world, position);
    position = mul(c_view, position);
    return mul(c_projection, position);
}
