#include "EnginePCH.h"
#include "ShadowPass.h"
#include "RenderThreadHandler.h"

//ID3D11DepthStencilView*& ShadowPass::EmplaceInMap(const unsigned int& index)
//{
//	// TODO: insert return statement here
//}

ShadowPass::ShadowPass()
{
}

ComPtr<ID3D11DepthStencilView> ShadowPass::CreateDepthView(uint32_t index)
{

	ComPtr<ID3D11DepthStencilView> depthView;
	D3D11_DEPTH_STENCIL_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	desc.Texture2DArray.MipSlice = 0;
	desc.Texture2DArray.ArraySize = 1;
	desc.Texture2DArray.FirstArraySlice = index;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	
	if (FAILED(D3D11Core::Get().Device()->CreateDepthStencilView(m_shadowMap.shadowTexture.Get(), &desc, depthView.GetAddressOf()))) 
	{
		LOG_WARNING("Failed to create Depth Stencil View");
	}
	
	return depthView;
}

ShadowPass::~ShadowPass()
{
}

ComPtr<ID3D11Buffer> ShadowPass::CreateLightBuffer(light_t light)
{
	ComPtr<ID3D11Buffer> buffer;

	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.ByteWidth = sizeof(camera_Matrix_t);
	desc.StructureByteStride = 0;

	camera_Matrix_t mat = {};
	switch (light.type)
	{
	case TypeLight::DIRECTIONAL:
	{
		mat.view = dx::XMMatrixLookToLH(light.position, light.direction, sm::Vector3::Up);
		mat.projection = dx::XMMatrixOrthographicLH(1000, 600, 1.f, 100.0f);
		break;
	}
	case TypeLight::POINT:
	{
		mat.view = dx::XMMatrixLookToLH(light.position, light.direction, sm::Vector3::Up);
		mat.projection = dx::XMMatrixOrthographicLH(1000, 600, 1.f, 100.0f);
		break;
	}
	default:
		break;
	}

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &mat;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	if (FAILED(D3D11Core::Get().Device()->CreateBuffer(&desc, &data, buffer.GetAddressOf())))
	{
		LOG_WARNING("Failed to create Light buffer");
	}

	return buffer;

}

void ShadowPass::SetupMap(uint32_t arraySize)
{
	if (arraySize <= 0)
		return;

	if (m_shadowMap.shadowTexture)
	{
		m_shadowMap.shadowTexture->Release();
		m_shadowMap.shadowTexture = nullptr;
	}
	if (m_shadowMap.shadowView)
	{
		m_shadowMap.shadowView->Release();
		m_shadowMap.shadowView = nullptr;
	}

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = SHADOW_SIZE;
	texDesc.Height = SHADOW_SIZE;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = (UINT)arraySize;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	HRESULT hr = D3D11Core::Get().Device()->CreateTexture2D(&texDesc, NULL, &m_shadowMap.shadowTexture);
	if (FAILED(hr))
	{
		LOG_WARNING("Failed setting up shadows, No shadows will be drawn.");
	}
}

void ShadowPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
	if (pDeviceContext == D3D11Core::Get().DeviceContext())
	{
		const auto& lights = m_lights->GetLights();
		if (m_shadowMap.amount != lights.size())
		{
			SetupMap(lights.size());
			m_shadows.clear();
			for (unsigned int i = 0; i < static_cast<unsigned int>(lights.size()); i++)
			{
				ShadowSection section;
				section.shadowDepth = this->CreateDepthView(i);
				section.lightBuffer = this->CreateLightBuffer(lights[i]);
				m_shadows.push_back(section);
			}
			m_shadowMap.amount = lights.size();
		}
	}


}

void ShadowPass::Render(Scene* pScene)
{
	const render_instructions_t inst = thread::RenderThreadHandler::Get().DoShadows(m_shadowMap.amount);

	/*
		Everything renders on one single thread.
	*/
	if ((inst.start | inst.stop) == 0)
	{
		for (auto& shadow : m_shadows)
		{
			ID3D11RenderTargetView* nullTargets[8] = { nullptr };
			D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(1, 1, shadow.lightBuffer.GetAddressOf());
			D3D11Core::Get().DeviceContext()->OMSetRenderTargets(8, nullTargets, shadow.shadowDepth.Get());


		}
	}
	else
		// Main thread renders last part.
	{

	}
}

void ShadowPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{

	//DC->PSSetShaderResources(0, )


}
