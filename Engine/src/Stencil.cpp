#include "EnginePCH.h"
#include "Stencil.h"

Stencil::Stencil()
{
	
}

void Stencil::Initialize(MODE mode)
{
	D3D11_DEPTH_STENCIL_DESC desc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
	if (mode == MODE::WRITE)
	{
		desc.StencilEnable = true;
		desc.StencilWriteMask = 0xFF;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_REPLACE;
	}
	else if (mode == MODE::MASK)
	{
		desc.DepthEnable = false;
		desc.StencilEnable = true;
		desc.StencilReadMask = 0xFF;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	}

	D3D11Core::Get().Device()->CreateDepthStencilState(&desc, m_depthStencil.GetAddressOf());
}

Stencil::~Stencil()
{
}

void Stencil::Bind()
{
	D3D11Core::Get().DeviceContext()->OMSetDepthStencilState(m_depthStencil.Get(), 0);

}
