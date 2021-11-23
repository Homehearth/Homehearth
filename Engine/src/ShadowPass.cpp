#include "EnginePCH.h"
#include "ShadowPass.h"
#include "RenderThreadHandler.h"

//ID3D11DepthStencilView*& ShadowPass::EmplaceInMap(const unsigned int& index)
//{
//	// TODO: insert return statement here
//}

ShadowPass::ShadowPass()
{
	m_viewport.Height = SHADOW_SIZE;
	m_viewport.Width = SHADOW_SIZE;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

	// Direct3D uses a depth buffer range of 0 to 1, hence:
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
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
	desc.CPUAccessFlags = 0;
	desc.ByteWidth = sizeof(camera_Matrix_t);
	desc.StructureByteStride = 0;

	camera_Matrix_t mat = this->GetLightMatrix(light);

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

camera_Matrix_t ShadowPass::GetLightMatrix(light_t light)
{
	camera_Matrix_t mat = {};
	switch (light.type)
	{
	case TypeLight::DIRECTIONAL:
	{
		mat.view = dx::XMMatrixLookToLH(light.position, light.direction, sm::Vector3::Up);
		mat.projection = dx::XMMatrixOrthographicLH(SHADOW_SIZE / 30, SHADOW_SIZE / 30, 0.f, 500.0f);
		break;
	}
	case TypeLight::POINT:
	{
		/*
		mat.view = dx::XMMatrixLookToLH(light.position, light.direction, sm::Vector3::Up);
		mat.projection = dx::XMMatrixPerspectiveFovLH(dx::XMConvertToRadians(90), 1.f, 1.f, 100.0f);
		*/

		break;
	}
	default:
		break;
	}

	return mat;
}

void ShadowPass::UpdateLightBuffer(ID3D11DeviceContext* context, ID3D11Buffer* buffer, light_t light)
{
	camera_Matrix_t mat = this->GetLightMatrix(light);
	context->UpdateSubresource(buffer, 0, nullptr, &mat, 0, 0);
}


void ShadowPass::SetupMap(uint32_t arraySize)
{
	if (arraySize <= 0)
		return;

	m_shadowMap.shadowTexture = nullptr;
	m_shadowMap.shadowView = nullptr;

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

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.ArraySize = arraySize;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.MostDetailedMip = 0;

	hr = D3D11Core::Get().Device()->CreateShaderResourceView(m_shadowMap.shadowTexture.Get(), &srvDesc, m_shadowMap.shadowView.GetAddressOf());
	if (FAILED(hr))
	{
		LOG_WARNING("Failed setting up shadows, No shadows will be drawn.");
	}
}

void ShadowPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
	if (pDeviceContext == D3D11Core::Get().DeviceContext())
	{
		auto& lights = m_lights->GetLights();
		if (m_shadowMap.amount != lights.size())
		{
			SetupMap(lights.size());
			m_shadows.clear();
			for (unsigned int i = 0; i < static_cast<unsigned int>(lights.size()); i++)
			{
				ShadowSection section;
				section.shadowDepth = this->CreateDepthView(i);
				section.lightBuffer = this->CreateLightBuffer(lights[i]);
				section.pLight = &lights[i];
				m_shadows.push_back(section);
			}
			m_shadowMap.amount = lights.size();
		}
	}

	ID3D11ShaderResourceView* nullViews[] = { nullptr };
	DC->PSSetShaderResources(13, 1, nullViews);

	DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DC->IASetInputLayout(PM->m_defaultInputLayout.Get());

	DC->PSSetSamplers(1, 1, PM->m_linearSamplerState.GetAddressOf());
	DC->RSSetViewports(1, &m_viewport);
	DC->RSSetState(PM->m_rasterStateNoCulling.Get());

	DC->VSSetShader(PM->m_defaultVertexShader.Get(), nullptr, 0);
	DC->PSSetShader(PM->m_shadowPixelShader.Get(), nullptr, 0);

	DC->OMSetBlendState(PM->m_blendStateParticle.Get(), 0, UINT32_MAX);
}

void ShadowPass::Render(Scene* pScene)
{
	PROFILE_FUNCTION();
	thread::RenderThreadHandler::Get().SetObjectsBuffer(&pScene->m_renderableCopies);
	thread::RenderThreadHandler::Get().SetShadows(&m_shadows);
	const render_instructions_t inst = thread::RenderThreadHandler::Get().DoShadows(m_shadowMap.amount);
	//const render_instructions_t inst;

	/*
		Everything renders on one single thread.
	*/
	if ((inst.start | inst.stop) == 0)
	{
		for (auto& shadow : m_shadows)
		{
			const auto& lights = m_lights->GetLights();
			this->UpdateLightBuffer(D3D11Core::Get().DeviceContext(), shadow.lightBuffer.Get(), *shadow.pLight);
			D3D11Core::Get().DeviceContext()->ClearDepthStencilView(shadow.shadowDepth.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
			ID3D11RenderTargetView* nullTargets[8] = { nullptr };
			D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(1, 1, shadow.lightBuffer.GetAddressOf());
			D3D11Core::Get().DeviceContext()->OMSetRenderTargets(8, nullTargets, shadow.shadowDepth.Get());
			pScene->RenderShadow(*shadow.pLight);

			D3D11Core::Get().DeviceContext()->OMSetRenderTargets(8, nullTargets, nullptr);

		}
	}
	else
		// Main thread renders last part.
	{
		for (int i = inst.start; i < inst.stop; i++)
		{
			const ShadowSection& shadow = m_shadows[i];
			const auto& lights = m_lights->GetLights();
			this->UpdateLightBuffer(D3D11Core::Get().DeviceContext(), shadow.lightBuffer.Get(), *shadow.pLight);
			D3D11Core::Get().DeviceContext()->ClearDepthStencilView(shadow.shadowDepth.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
			ID3D11RenderTargetView* nullTargets[8] = { nullptr };
			D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(1, 1, shadow.lightBuffer.GetAddressOf());
			D3D11Core::Get().DeviceContext()->OMSetRenderTargets(8, nullTargets, shadow.shadowDepth.Get());
			pScene->RenderShadow(*shadow.pLight);

			D3D11Core::Get().DeviceContext()->OMSetRenderTargets(8, nullTargets, nullptr);
		}

		thread::RenderThreadHandler::Get().ExecuteCommandLists();
	}

}

void ShadowPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
	
	DC->PSSetShaderResources(13, 1, m_shadowMap.shadowView.GetAddressOf());


}
