#include "EnginePCH.h"
#include "Camera.h"

Camera::Camera(sm::Vector3 pos, sm::Vector3 target, sm::Vector3 up, sm::Vector2 windowSize)
    :m_position(pos), m_target(target), m_up(up), m_windowHeight(windowSize.y), m_windowWidth(windowSize.x), m_rollPitchYaw(0,0,0)
{
    m_FOV = dx::XMConvertToRadians(90.f); //0.4f * 3.14f;
    m_zoomValue = 1;
    m_aspectRatio = m_windowWidth / m_windowHeight;
    m_nearPlane = 0.01f; // 1.0f;
    m_farPlane = 100.0f; // 1000.0f

    m_view = dx::XMMatrixLookAtRH(m_position, m_target, m_up);
    m_projection = dx::XMMatrixPerspectiveFovLH(m_FOV * m_zoomValue, m_aspectRatio, m_nearPlane, m_farPlane);
}

void Camera::Update(float deltaTime)
{
    m_rotationMatrix = dx::XMMatrixRotationRollPitchYaw(m_rollPitchYaw.y, m_rollPitchYaw.z, m_rollPitchYaw.x);

    quaterion = sm::Quaternion::CreateFromYawPitchRoll(m_rollPitchYaw.z, m_rollPitchYaw.y, m_rollPitchYaw.x);

    //Update camera values
    m_right = dx::XMVector3TransformCoord(m_defaultRight, m_rotationMatrix);
    m_forward = dx::XMVector3TransformCoord(m_defaultForward, m_rotationMatrix);
    m_up = dx::XMVector3Cross(m_forward, m_right);

    m_target = m_position + m_target;
    m_view = dx::XMMatrixLookAtRH(m_position, m_target, m_up);
}

sm::Matrix Camera::GetView()
{
    return m_view;
}

sm::Matrix Camera::GetProjection()
{
    return m_projection;
}

sm::Vector3 Camera::GetPosition()
{
    return m_position;
}

sm::Vector3 Camera::GetTarget()
{
    return m_target;
}

void Camera::SetFOV(float fov)
{
    m_FOV = fov;
}

void Camera::SetNearFarPlane(float nearPlane, float farPlane)
{
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
}

void Camera::SetNearPlane(float nearPlane)
{
    m_nearPlane = nearPlane;
}

void Camera::SetFarPlane(float farPlane)
{
    m_farPlane = farPlane;
}

void Camera::SetZoom(float val)
{
    m_zoomValue = val;
}

void Camera::UpdateProjection()
{
    m_projection = dx::XMMatrixPerspectiveFovLH(m_FOV * m_zoomValue, m_aspectRatio, m_nearPlane, m_farPlane);
}