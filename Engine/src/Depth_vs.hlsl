cbuffer Matrices : register(b0)
{
    float4x4 world;
}

cbuffer Camera : register(b1)
{
    float4x4 projectionView;
}

// Depth pre-pass.
float4 main( float4 pos : POSITION ) : SV_POSITION
{
    pos = mul(world, pos);
    pos = mul(projectionView, pos);
	
	return pos;
}
