#include "EnginePCH.h"
#include "Camera.h"

Camera::Camera(sm::Vector3 pos, sm::Vector3 target, sm::Vector3 up, sm::Vector2 windowSize)
    :m_position(pos), m_target(target), m_up(up), m_windowHeight(windowSize.x), m_windowWidth(windowSize.y)
{
    m_FOV = 0.4f * 3.14f;
    m_aspectRatio = m_windowWidth / m_windowHeight;
    m_nearPlane = 1.0f;
    m_farPlane = 1000.0f;

    m_view = dx::XMMatrixLookAtLH(m_position, m_target, m_up);
    m_projection = dx::XMMatrixPerspectiveFovLH(m_FOV, m_windowWidth / m_windowHeight, m_nearPlane, m_farPlane);
}

sm::Matrix Camera::GetView()
{
    return m_view;
}

sm::Matrix Camera::GetProjection()
{
    return m_projection;
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

void Camera::UpdateProjection()
{
    m_projection = dx::XMMatrixPerspectiveFovLH(m_FOV, m_windowWidth / m_windowHeight, m_nearPlane, m_farPlane);
}