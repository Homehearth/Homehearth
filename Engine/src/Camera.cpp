#include "EnginePCH.h"
#include "Camera.h"

#define KEYPRESS InputSystem::Get().CheckKeyboardKey

Camera::Camera()
{
    m_FOV = dx::XMConvertToRadians(90.f); //0.4f * 3.14f;
    m_zoomValue = 1;
    m_nearPlane = 0.01f; // 1.0f;
    m_farPlane = 100.0f; // 1000.0f
    m_rollPitchYaw = { 0.0f, 0.0f, 0.0f };
    m_move = { 0.0f, 0.0f, 0.0f };
    m_aspectRatio = 0;
    m_windowHeight = 0;
    m_windowWidth = 0;
    m_rotationSpeed = 5.0f;
    m_movingSepeed = 15.0f;
    m_targetVelocity = NULL;
    m_type = CAMERATYPE::DEFAULT;
}

Camera::~Camera()
{

}

void Camera::Initialize(sm::Vector3 pos, sm::Vector3 target, sm::Vector3 up, sm::Vector2 windowSize, CAMERATYPE type)
{
    m_position = pos;
    m_target = target;
    m_forward = dx::XMVector3Normalize(m_target);
    m_up = up;
    m_windowHeight = windowSize.y;
    m_windowWidth = windowSize.x;
    m_aspectRatio = m_windowWidth / m_windowHeight;
    m_type = type;

    m_defaultForward = { 0.0f, 0.0f, -1.0f };
    m_defaultRight = { -1.0f, 0.0f, 0.0f };

    m_view = dx::XMMatrixLookAtLH(m_position, m_target, m_up);
    m_projection = dx::XMMatrixPerspectiveFovLH(m_FOV * m_zoomValue, m_aspectRatio, m_nearPlane, m_farPlane);

    //Constant buffer struct
    m_cameraMat.position = sm::Vector4( m_position.x, m_position.y, m_position.z, 0.0f );
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
        m_currentMousePosition = sm::Vector2((float)InputSystem::Get().GetMousePos().x, (float)InputSystem::Get().GetMousePos().y);

        if (m_currentMousePosition.x != m_lastMousePosition.x || m_currentMousePosition.y != m_lastMousePosition.y)
        {
            m_rollPitchYaw.z += m_lastMousePosition.x * m_rotationSpeed * deltaTime;
            m_rollPitchYaw.y -= m_lastMousePosition.y * m_rotationSpeed * deltaTime;
            m_lastMousePosition = m_currentMousePosition;
        }

        quaterion = sm::Quaternion::CreateFromYawPitchRoll(m_rollPitchYaw.z, m_rollPitchYaw.y, m_rollPitchYaw.x);
        m_rotationMatrix = dx::XMMatrixRotationRollPitchYaw(m_rollPitchYaw.y, m_rollPitchYaw.z, m_rollPitchYaw.x);

        //Keyboard
        if (KEYPRESS(dx::Keyboard::E, KeyState::HELD)) //Down
        {
            m_move.y -= m_movingSepeed * deltaTime;
        }
        if (KEYPRESS(dx::Keyboard::Q, KeyState::HELD)) //UP
        {
            m_move.y += m_movingSepeed * deltaTime;
        }
        m_move.x = -InputSystem::Get().GetAxis(Axis::HORIZONTAL) * m_movingSepeed * deltaTime;
        m_move.z = -InputSystem::Get().GetAxis(Axis::VERTICAL) * m_movingSepeed * deltaTime;

        //Update camera values
        m_right = dx::XMVector3TransformNormal(m_defaultRight, m_rotationMatrix);
        m_forward = dx::XMVector3TransformNormal(m_defaultForward, m_rotationMatrix);

        m_target = dx::XMVector3TransformCoord(m_defaultForward, m_rotationMatrix);
        m_target = dx::XMVector3Normalize(m_target);

        m_up = dx::XMVector3Cross(m_forward, m_right);
        m_up = dx::XMVector3Normalize(m_up);

        m_move = sm::Vector3::Transform(m_move, quaterion);

        m_position += m_move;
        m_move = { 0.0f, 0.0f, 0.0f };
        m_forward = m_target;

        m_target = dx::XMVectorAdd(m_target, m_position);
        m_view = dx::XMMatrixLookAtLH(m_position, m_target, m_up);
    }
    else if (m_type == CAMERATYPE::PLAY)
    {
        m_position.x += m_targetVelocity->vel.x * deltaTime;
        m_position.z += m_targetVelocity->vel.z * deltaTime;

        m_right = dx::XMVector3TransformNormal(m_defaultRight, m_rotationMatrix);
        m_forward = dx::XMVector3TransformNormal(m_defaultForward, m_rotationMatrix);

        m_target = dx::XMVector3TransformCoord(m_defaultForward, m_rotationMatrix);

        m_up = dx::XMVector3Cross(m_forward, m_right);
        m_up = dx::XMVector3Normalize(m_up);

        m_move = sm::Vector3::Transform(m_move, quaterion);

        m_move = { 0.0f, 0.0f, 0.0f };
        m_forward = m_target;

        m_target = dx::XMVectorAdd(m_target, m_position);
        m_view = dx::XMMatrixLookAtLH(m_position, m_target, m_up);

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

void Camera::SetFollowVelocity(comp::Velocity* target)
{
    m_targetVelocity = target;
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

CAMERATYPE Camera::GetCameraType()
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

void Camera::UpdateProjection()
{
    m_projection = dx::XMMatrixPerspectiveFovLH(m_FOV * m_zoomValue, m_aspectRatio, m_nearPlane, m_farPlane);
}