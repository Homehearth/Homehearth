#include "EnginePCH.h"
#include "Camera.h"

#define KEYPRESS InputSystem::Get().CheckKeyboardKey

Camera::Camera()
{
	m_FOV				= dx::XMConvertToRadians(90.f); //0.4f * 3.14f;
	m_zoomValue			= 1;
	m_nearPlane			= 0.01f; // 1.0f;
	m_farPlane			= 1000.0; // 1000.0f
	m_rollPitchYaw		= { 0.0f, 0.0f, 0.0f };
	m_move				= { 0.0f, 0.0f, 0.0f };
	m_aspectRatio		= 0;
	m_windowHeight		= 0;
	m_windowWidth		= 0;
	m_rotationSpeed		= 2.5f;
	m_movingSpeed		= 15.0f;
	m_type				= CAMERATYPE::DEFAULT;
}

Camera::~Camera()
{

}

void Camera::Initialize(sm::Vector3 pos, sm::Vector3 target, sm::Vector3 up, sm::Vector2 windowSize, CAMERATYPE type)
{
	m_defaultPos = pos;
	m_position = pos;
	m_target = target;
	m_forward = dx::XMVector3Normalize(m_target - m_position);
	m_up = dx::XMVector3Normalize(up);
	m_windowHeight = windowSize.y;
	m_windowWidth = windowSize.x;
	m_aspectRatio = m_windowWidth / m_windowHeight;
	m_type = type;

	m_defaultForward = m_forward;
	m_defaultRight = dx::XMVector3Normalize(dx::XMVector3Cross(m_up, m_forward));

	m_view = dx::XMMatrixLookAtLH(m_position, m_target, m_up);
	m_projection = dx::XMMatrixPerspectiveFovLH(m_FOV * m_zoomValue, m_aspectRatio, m_nearPlane, m_farPlane);

	//Constant buffer struct
	m_cameraMat.position = sm::Vector4(m_position.x, m_position.y, m_position.z, 0.0f);
	m_cameraMat.target = sm::Vector4(m_target.x, m_target.y, m_target.z, 0);
	m_cameraMat.projection = m_projection;
	m_cameraMat.view = m_view;

	//Constant buffer
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeof(camera_Matrix_t);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &m_cameraMat;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&desc, &data, m_viewConstantBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		LOG_ERROR("Camera couldnt create constant buffer");
	}
}

void Camera::Update(float deltaTime)
{

	if (m_type == CAMERATYPE::DEBUG && InputSystem::Get().IsMouseRelative()) //Can't move the camera when in absolut mode
	{
		//Mouse
		m_rollPitchYaw.z += (float)InputSystem::Get().GetMousePos().x * m_rotationSpeed * deltaTime;
		m_rollPitchYaw.y += (float)InputSystem::Get().GetMousePos().y * m_rotationSpeed * deltaTime;

		quaterion = sm::Quaternion::CreateFromYawPitchRoll(m_rollPitchYaw.z, m_rollPitchYaw.y, m_rollPitchYaw.x);
		m_rotationMatrix = dx::XMMatrixRotationRollPitchYaw(m_rollPitchYaw.y, m_rollPitchYaw.z, m_rollPitchYaw.x);

		//Keyboard
		if (KEYPRESS(dx::Keyboard::E, KeyState::HELD)) //Down
		{
			m_move.y -= m_movingSpeed * deltaTime;
		}
		if (KEYPRESS(dx::Keyboard::Q, KeyState::HELD)) //UP
		{
			m_move.y += m_movingSpeed * deltaTime;
		}
		m_move.x = static_cast<float>(InputSystem::Get().GetAxis(Axis::HORIZONTAL));
		m_move.z = static_cast<float>(InputSystem::Get().GetAxis(Axis::VERTICAL));

		//Update camera values
		m_right = dx::XMVector3TransformNormal(m_defaultRight, m_rotationMatrix);
		m_forward = dx::XMVector3TransformNormal(m_defaultForward, m_rotationMatrix);

		m_target = dx::XMVector3TransformCoord(m_defaultForward, m_rotationMatrix);
		m_target = dx::XMVector3Normalize(m_target);

		m_up = dx::XMVector3Cross(m_forward, m_right);
		m_up = dx::XMVector3Normalize(m_up);

		m_move = sm::Vector3::Transform(m_move, quaterion);

		m_position += m_move * m_movingSpeed * deltaTime;
		m_move = { 0.0f, 0.0f, 0.0f };
		m_forward = m_target;

		m_target = dx::XMVectorAdd(m_target, m_position);
		m_view = dx::XMMatrixLookAtLH(m_position, m_target, m_up);
	}
	else if (m_type == CAMERATYPE::PLAY)
	{
		quaterion = sm::Quaternion::CreateFromYawPitchRoll(m_rollPitchYaw.z, m_rollPitchYaw.y, m_rollPitchYaw.x);
		m_rotationMatrix = dx::XMMatrixRotationRollPitchYaw(m_rollPitchYaw.y, m_rollPitchYaw.z, m_rollPitchYaw.x);

		comp::Transform* targetTransform = nullptr;
		
		if (!m_targetEntity.IsNull())
		{
			targetTransform = m_targetEntity.GetComponent<comp::Transform>();
		}

		if (targetTransform)
		{
			m_position = m_defaultPos + targetTransform->position;
		}
			
		m_right = dx::XMVector3TransformNormal(m_defaultRight, m_rotationMatrix);
		m_forward = dx::XMVector3TransformNormal(m_defaultForward, m_rotationMatrix);

		m_up = dx::XMVector3Cross(m_forward, m_right);
		m_up = dx::XMVector3Normalize(m_up);

		if (targetTransform)
		{
			m_target = targetTransform->position - m_position;
		}
		else
		{
			m_target = m_position + m_defaultForward;
		}
		m_view = dx::XMMatrixLookToLH(m_position, m_target, m_up);

		UpdateProjection();
	}
	else if (m_type == CAMERATYPE::DEFAULT)
	{
		//TODO: check if see if something is needed to add
	}

	m_cameraMat.position = { m_position.x, m_position.y, m_position.z, 0.0f };
	m_cameraMat.target = { m_target.x, m_target.y, m_target.z, 0 };
	m_cameraMat.projection = m_projection;
	m_cameraMat.view = m_view;
}

void Camera::SetFollowEntity(const Entity& entity)
{
	m_targetEntity = entity;
}


//Get functions
sm::Matrix Camera::GetView() const
{
	return m_view;
}

sm::Matrix Camera::GetProjection() const
{
	return m_projection;
}

sm::Vector3 Camera::GetPosition() const
{
	return m_position;
}

sm::Vector3 Camera::GetTarget() const
{
	return m_target;
}

sm::Vector3 Camera::GetUp() const
{
	return m_up;
}

camera_Matrix_t* Camera::GetCameraMatrixes()
{
	return &m_cameraMat;
}

CAMERATYPE Camera::GetCameraType()const
{
	return m_type;
}

void Camera::SetPosition(sm::Vector3 newPosition)
{
	m_position = { newPosition.x, newPosition.y, newPosition.z };
}

//Set functions
void Camera::SetFOV(float fov)
{
	m_FOV = fov;
	UpdateProjection();
}

void Camera::SetNearFarPlane(float nearPlane, float farPlane)
{
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;
	UpdateProjection();
}

void Camera::SetNearPlane(float nearPlane)
{
	m_nearPlane = nearPlane;
	UpdateProjection();
}

void Camera::SetFarPlane(float farPlane)
{
	m_farPlane = farPlane;
	UpdateProjection();
}

void Camera::SetZoom(float val)
{
	m_zoomValue = val;
	UpdateProjection();
}

void Camera::SetRollPitchYaw(sm::Vector3 rotation)
{
	m_rollPitchYaw = rotation;
}

void Camera::UpdateProjection()
{
	m_projection = dx::XMMatrixPerspectiveFovLH(m_FOV * m_zoomValue, m_aspectRatio, m_nearPlane, m_farPlane);

}