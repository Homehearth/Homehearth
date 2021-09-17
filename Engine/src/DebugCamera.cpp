#include "EnginePCH.h"
#include "DebugCamera.h"

#define DEVICE D3D11Core::Get().Device()
#define CONTEXT D3D11Core::Get().DeviceContext()

Camera::Camera()
{
	this->cameraBuffer = nullptr;
	this->forward = { 0.0f, 0.0f, 1.0f };
	this->rotation = { 0.0001f, 0.0001f, 0.0001f, 0.0 };
}

Camera::~Camera()
{
	if (this->cameraBuffer)
		this->cameraBuffer->Release();
}

void Camera::Initialize(CamParts& cam)
{
	this->parts = cam;
	D3D11_BUFFER_DESC bDesc;
	bDesc.ByteWidth = sizeof(CamParts);
	bDesc.Usage = D3D11_USAGE_DYNAMIC;
	bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &this->parts;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	if (!this->cameraBuffer)
		DEVICE->CreateBuffer(&bDesc, &data, &this->cameraBuffer);
	else // if cambuffer already exists then release it and create a new one.
	{
		this->cameraBuffer->Release();
		DEVICE->CreateBuffer(&bDesc, &data, &this->cameraBuffer);
	}
}

void Camera::Preset()
{
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 proj;
	DirectX::XMFLOAT3 pos = { 1.0f, 0.00001f, -10.0f };
	DirectX::XMVECTOR pos_vec = DirectX::XMLoadFloat3(&pos);
	DirectX::XMVECTOR lookAt = { 0.0f, 0.0f, 1.0f };
	DirectX::XMStoreFloat4x4(&view, DirectX::XMMatrixTranspose(
		DirectX::XMMatrixLookAtLH(pos_vec, lookAt, up)));
	DirectX::XMStoreFloat4x4(&proj, DirectX::XMMatrixTranspose(
		DirectX::XMMatrixPerspectiveFovLH(0.5f * DirectX::XM_PI, (float)((1920 / 2.0f) / (1080 / 2.0f)), 0.3f, 300.0f)));

	CamParts camParts(view, proj, pos);
	this->parts = camParts;

	this->Initialize(camParts);
}

void Camera::Render()
{
	CONTEXT->VSSetConstantBuffers(1, 1, &this->cameraBuffer);
	//CONTEXT->PSSetConstantBuffers(1, 1, &this->cameraBuffer);
}

void Camera::Update(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 lookAt)
{
	CamParts newParts;
	newParts.position = pos;
	DirectX::XMStoreFloat4x4(&newParts.projection,
		DirectX::XMMatrixTranspose(
			DirectX::XMMatrixPerspectiveFovLH(0.5f * DirectX::XM_PI, (float)((1920 / 2.0f) / (1080 / 2.0f)), 0.3f, 300.0f)));

	DirectX::XMStoreFloat4x4(&newParts.view,
		DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&lookAt), up)));

	this->parts = newParts;

	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	CONTEXT->Map(this->cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &this->parts, sizeof(CamParts));
	CONTEXT->Unmap(this->cameraBuffer, 0);
}

void Camera::UpdateDOOM()
{

	DirectX::XMMATRIX camRotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(this->rotation.x, this->rotation.y, this->rotation.z);
	DirectX::XMVECTOR camTarget = DirectX::XMVector3TransformCoord(_forward, camRotationMatrix);
	DirectX::XMStoreFloat3(&this->forward, camTarget);

	camTarget = DirectX::XMVectorAdd(camTarget, DirectX::XMLoadFloat3(&this->parts.position));

	DirectX::XMVECTOR _up = DirectX::XMVector3TransformCoord(up, camRotationMatrix);

	DirectX::XMStoreFloat4x4(&this->parts.view, DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(
		DirectX::XMLoadFloat3(&this->parts.position), camTarget, _up)));

	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	CONTEXT->Map(this->cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &this->parts, sizeof(CamParts));
	CONTEXT->Unmap(this->cameraBuffer, 0);
}

CamParts& Camera::GetParts()
{
	return this->parts;
}

DirectX::XMFLOAT3 Camera::CameraForward() const
{
	return this->forward;
}

DirectX::XMFLOAT4& Camera::CamRotation()
{
	return this->rotation;
}
