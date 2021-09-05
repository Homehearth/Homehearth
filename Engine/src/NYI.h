#pragma once

/*	Not Yet Implemented
 *
 *	Just a few ideas by Jonathan.

namespace Graphics
{
    class Window;
    class Device;
    class SwapChain;
    class Adapter;
}

enum class ETextureType : uint32_t { Ambient, Emissive, Diffuse, Specular, Normal, Opacity, AmbientOcclussion, Roughness, Height, Depth, DepthStencil, TEXTURE_COUNT };
enum class EBufferType : uint32_t { ConstantBuffer, VertexBuffer, IndexBuffer, BUFFER_COUNT };
enum class EShaderType : uint32_t { VertexShader, PixelShader, ComputeShader, GeometryShader, HullShader, SHADER_COUNT };
enum class ELightType: uint32_t { PointLight, SpotLight, DirectionalLight, LIGHT_COUNT };
enum class EResourceView : uint32_t { ShaderResourceView, RenderTargetView, DepthStencilView, UnorderedAccessView, VIEW_COUNT };

struct per_frame_t {};
struct per_object_t {};
struct quad_t {};

struct material_properties_t
{
    GlobalAmbient, AmbientColor,
    EmissiveColor, DiffuseColor,
    SpecularColor, Reflectance,
    Opacity, SpecularPower,
    IndexOfRefraction, HasAmbientTexture,
    HasEmissiveTexture, HasDiffuseTexture,
    HasSpecularTexture, HasSpecularPowerTexture,
    HasNormalTexture, HasBumpTexture, HasOpacityTexture,
    BumpIntensity, SpecularScale, AlphaThreshold,
};

struct pipeline_state_t
{
	ShaderID		    shader;
	BufferID		    vertexBuffer;
	BufferID		    indexBuffer;
	EPrimitiveTopology	topology;
	Viewport		    viewPort;
	RasterizerStateID	rasterizerState;
	DepthStencilStateID	depthStencilState;
	BlendStateID		blendState;
	RenderTargetIDs		renderTargets;
	DepthTargetID		depthTargets;
};

struct simple_vertex_t
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT3 bitangent;
	DirectX::XMFLOAT2 texcoord;
};

struct texture_resource_t
{
	ComPtr<ID3D11Texture2D> texture;
	ComPtr<ID3D11RenderTargetView> renderTargetView;
	ComPtr<ID3D11ShaderResourceView> shaderResourceView;
};

 */