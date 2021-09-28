#pragma once
#include "EnginePCH.h"

class Camera
{
private:
	sm::Vector3 m_position;
	sm::Vector3 m_forward;
	sm::Vector3 m_right;
	sm::Vector3 m_up;
	sm::Vector3 m_target;
	sm::Vector3 m_rollPitchYaw;

	sm::Matrix  m_view;
	sm::Matrix  m_projection;
	sm::Matrix  m_rotation;

	float m_FOV;
	float m_nearPlane;
	float m_farPlane;
	float m_windowHeight;
	float m_windowWidth;
	float m_aspectRatio;

	sm::Quaternion quaterion;

public:
	/* Position, Target, up, windowSize = (window height, window width) */
	Camera(sm::Vector3 pos, sm::Vector3 target, sm::Vector3 up, sm::Vector2 windowSize);
	void Update(float deltaTime);

	//Get Functions
	sm::Matrix GetView();
	sm::Matrix GetProjection();
	sm::Vector3 GetPosition();
	sm::Vector3 GetTarget();
	
	//Set Functions
	void SetFOV(float fov);
	void SetFocalLength();
	void SetNearFarPlane(float nearPlane, float farPlane);
	void SetNearPlane(float nearPlane);
	void SetFarPlane(float farPlane);
	/*Run this function after setting any new values for projection*/
	void UpdateProjection();
};

