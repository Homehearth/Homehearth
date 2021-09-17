#pragma once
#include <DirectXMath.h>
#include <d3d11.h>

/*
	Contains the view, projection and position of camera.
*/
struct CamParts
{
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;

	DirectX::XMFLOAT3 position;
	float pad;
	DirectX::XMFLOAT3 lookAt;
	float pad1;

	CamParts(DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4X4 projection,
		DirectX::XMFLOAT3 position)
	{
		this->view = view;
		this->projection = projection;
		this->position = position;
		this->lookAt = {};

		this->pad = 0;
		this->pad1 = 0;
	}

	CamParts()
	{
		this->view = {};
		this->projection = {};
		this->position = {};
		this->lookAt = {};
		this->pad = 0;
		this->pad1 = 0;
	}
};

/*
	To create a camera object to be used for rendering, Use this class.
	For specifications such as view, projection and position of camera see
	attached CamParts struct.
*/
class Camera
{
private:

	ID3D11Buffer* cameraBuffer;

	CamParts parts;
	DirectX::XMFLOAT3 forward;
	DirectX::XMFLOAT4 rotation;

public:

	Camera();
	~Camera();

	// Initializes the camera with specified camParts.
	void Initialize(CamParts& cam);

	// Init the camera with a preset camPart specification.
	void Preset();

	// used inside engine, ignore this function.
	void Render();

	// update the camera with new pos and look at position.
	void Update(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 lookAt);

	// Update the camera doom-style.
	void UpdateDOOM();

	CamParts& GetParts();
	DirectX::XMFLOAT3 CameraForward() const;
	DirectX::XMFLOAT4& CamRotation();
};

constexpr DirectX::XMVECTOR up = { 0.0f, 1.0f, 0.0f, 0.0f };
constexpr DirectX::XMVECTOR _forward = { 0.0f, 0.0f, 1.0f, 0.0f };
