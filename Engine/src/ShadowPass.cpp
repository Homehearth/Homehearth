#include "EnginePCH.h"
#include "ShadowPass.h"
#include "RenderThreadHandler.h"

ShadowPass::ShadowPass()
{
	m_shadowSize = 2048;


	m_viewport.Height = m_shadowSize;
	m_viewport.Width = m_shadowSize;
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

	camera_Matrix_t mat = this->GetLightMatrix(light, sm::Vector3::Down);

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

void ShadowPass::RenderWithImmidiateContext(Scene* pScene, const ShadowSection& shadow)
{
	if (!shadow.pLight->enabled)
		return;

	D3D11Core::Get().DeviceContext()->ClearDepthStencilView(shadow.shadowDepth[0].Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(1, 1, shadow.lightBuffer.GetAddressOf());
	ID3D11RenderTargetView* nullTargets[8] = { nullptr };
	D3D11Core::Get().DeviceContext()->OMSetRenderTargets(8, nullTargets, shadow.shadowDepth[0].Get());
	switch (shadow.pLight->type)
	{
	case TypeLight::DIRECTIONAL:
	{
		this->UpdateLightBuffer(D3D11Core::Get().DeviceContext(), shadow.lightBuffer.Get(), *shadow.pLight, sm::Vector3(shadow.pLight->direction));
		D3D11Core::Get().DeviceContext()->VSSetShader(PM->m_defaultVertexShader.Get(), nullptr, 0);
		pScene->RenderShadow();
		break;
	}
	case TypeLight::POINT:
	{
		D3D11Core::Get().DeviceContext()->ClearDepthStencilView(shadow.shadowDepth[1].Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		D3D11Core::Get().DeviceContext()->VSSetShader(PM->m_paraboloidVertexShader.Get(), nullptr, 0);

		this->UpdateLightBuffer(D3D11Core::Get().DeviceContext(), shadow.lightBuffer.Get(), *shadow.pLight, sm::Vector3::Down);
		pScene->RenderShadow();

		D3D11Core::Get().DeviceContext()->OMSetRenderTargets(8, nullTargets, shadow.shadowDepth[1].Get());
		
		this->UpdateLightBuffer(D3D11Core::Get().DeviceContext(), shadow.lightBuffer.Get(), *shadow.pLight, sm::Vector3::Up);
		pScene->RenderShadow();

		break;
	}
	default:
		break;
	}

	D3D11Core::Get().DeviceContext()->OMSetRenderTargets(8, nullTargets, nullptr);
}

camera_Matrix_t ShadowPass::GetLightMatrix(light_t light, sm::Vector3 direction)
{
	camera_Matrix_t mat = {};
	switch (light.type)
	{
	case TypeLight::DIRECTIONAL:
	{
		mat.view = dx::XMMatrixLookToLH(light.position, direction, sm::Vector3::Up);
		mat.projection = dx::XMMatrixOrthographicLH(500, 500, 0.1f, 400.0f);
		break;
	}
	case TypeLight::POINT:
	{
		mat.view = dx::XMMatrixLookToLH(light.position, direction, sm::Vector3::Forward);
		mat.projection = sm::Matrix::Identity; // projection is done in paraboloid vertex shader
		break;
	}
	default:
		break;
	}

	return mat;
}

void ShadowPass::UpdateLightBuffer(ID3D11DeviceContext* context, ID3D11Buffer* buffer, light_t light, sm::Vector3 direction)
{
	camera_Matrix_t mat = this->GetLightMatrix(light, direction);
	context->UpdateSubresource(buffer, 0, nullptr, &mat, 0, 0);
}

void ShadowPass::SetShadowMapSize(uint32_t size)
{
	m_shadowSize = size;
	m_viewport.Height = m_shadowSize;
	m_viewport.Width = m_shadowSize;

	SetupMap();
}

uint32_t ShadowPass::GetShadowMapSize() const
{
	return m_shadowSize;
}


void ShadowPass::CreateMap(uint32_t arraySize)
{
	if (arraySize <= 0)
		return;

	m_shadowMap.shadowTexture = nullptr;
	m_shadowMap.shadowView = nullptr;

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = m_shadowSize;
	texDesc.Height = m_shadowSize;
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

void ShadowPass::SetupMap()
{
	auto& lights = m_lights->GetLights();
	unsigned int arraySize = this->GetLightCount(TypeLight::DIRECTIONAL) + 2 * this->GetLightCount(TypeLight::POINT);
	CreateMap(arraySize);
	m_shadows.clear();
	unsigned int shadowIndex = 0;
	for (unsigned int i = 0; i < static_cast<unsigned int>(lights.size()); i++)
	{
		ShadowSection section;
		section.shadowDepth[0] = this->CreateDepthView(shadowIndex);
		section.lightBuffer = this->CreateLightBuffer(lights[i]);
		section.pLight = &lights[i];
		lights[i].shadowIndex = shadowIndex; // this is somewhere changed back to 0 after this shadow pass

		if (section.pLight->type == TypeLight::POINT)
		{
			shadowIndex++;
			section.shadowDepth[1] = this->CreateDepthView(shadowIndex);
		}
		shadowIndex++;
		m_shadows.push_back(section);
	}
	m_shadowMap.amount = static_cast<unsigned int>(lights.size());
}

void ShadowPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
	if (pDeviceContext == D3D11Core::Get().DeviceContext())
	{
		auto& lights = m_lights->GetLights();
		if (m_shadowMap.amount != lights.size())
		{
			SetupMap();
		}
	}

	
	ID3D11ShaderResourceView* nullViews[] = { nullptr };
	DC->PSSetShaderResources(13, 1, nullViews);

	DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DC->IASetInputLayout(PM->m_defaultInputLayout.Get());

	DC->PSSetSamplers(1, 1, PM->m_linearSamplerState.GetAddressOf());
	DC->RSSetViewports(1, &m_viewport);
	DC->RSSetState(PM->m_rasterState.Get());

	DC->VSSetShader(PM->m_defaultVertexShader.Get(), nullptr, 0);
	DC->PSSetShader(PM->m_shadowPixelShader.Get(), nullptr, 0);

	m_lights->Render(DC);

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
		for (const auto& shadow : m_shadows)
		{
			RenderWithImmidiateContext(pScene, shadow);
		}
	}
	else
		// Main thread renders last part.
	{
		for (int i = inst.start; i < inst.stop; i++)
		{
			const ShadowSection& shadow = m_shadows[i];
			RenderWithImmidiateContext(pScene, shadow);
		}

		thread::RenderThreadHandler::Get().ExecuteCommandLists();
	}


	D3D11Core::Get().DeviceContext()->IASetInputLayout(PM->m_animationInputLayout.Get());
	D3D11Core::Get().DeviceContext()->VSSetShader(PM->m_animationVertexShader.Get(), nullptr, 0);
	D3D11Core::Get().DeviceContext()->RSSetViewports(1, &m_viewport);
	ID3D11RenderTargetView* nullTargets[8] = { nullptr };
	for (const auto& shadow : m_shadows)
	{
		if (!shadow.pLight->enabled)
			continue;

		D3D11Core::Get().DeviceContext()->VSSetConstantBuffers(1, 1, shadow.lightBuffer.GetAddressOf());
		D3D11Core::Get().DeviceContext()->OMSetRenderTargets(8, nullTargets, shadow.shadowDepth[0].Get());
		switch (shadow.pLight->type)
		{
		case TypeLight::DIRECTIONAL:
		{
			this->UpdateLightBuffer(D3D11Core::Get().DeviceContext(), shadow.lightBuffer.Get(), *shadow.pLight, sm::Vector3(shadow.pLight->direction));
			D3D11Core::Get().DeviceContext()->VSSetShader(PM->m_animationVertexShader.Get(), nullptr, 0);
			pScene->RenderShadowAnimation();

			break;
		}
		case TypeLight::POINT:
		{
			D3D11Core::Get().DeviceContext()->VSSetShader(PM->m_paraboloidAnimationVertexShader.Get(), nullptr, 0);

			this->UpdateLightBuffer(D3D11Core::Get().DeviceContext(), shadow.lightBuffer.Get(), *shadow.pLight, sm::Vector3::Down);
			pScene->RenderShadowAnimation();

			D3D11Core::Get().DeviceContext()->OMSetRenderTargets(8, nullTargets, shadow.shadowDepth[1].Get());

			this->UpdateLightBuffer(D3D11Core::Get().DeviceContext(), shadow.lightBuffer.Get(), *shadow.pLight, sm::Vector3::Up);
			pScene->RenderShadowAnimation();


			break;
		}
		default:
			break;
		}
	}
	D3D11Core::Get().DeviceContext()->OMSetRenderTargets(8, nullTargets, nullptr);


}

void ShadowPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
	DC->PSSetShaderResources(13, 1, m_shadowMap.shadowView.GetAddressOf());
}

void ShadowPass::ImGuiShowTextures() {
	ImGui::Image((void*)m_shadowMap.shadowView.Get(), ImVec2(400, 400));
}
