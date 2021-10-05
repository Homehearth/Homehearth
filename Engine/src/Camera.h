#pragma once
#include "EnginePCH.h"

class Camera
{
private:
	sm::Vector2 m_currentMousePosition;
	sm::Vector2 m_lastMousePosition;

	sm::Vector3 m_position;
	sm::Vector3 m_up;
	sm::Vector3 m_target;
	sm::Vector3 m_rollPitchYaw;
	sm::Vector3 m_forward;
	sm::Vector3 m_right;
	sm::Vector3 m_defaultForward;
	sm::Vector3 m_defaultRight;
	sm::Vector3 m_move;

	sm::Matrix  m_view;
	sm::Matrix  m_projection;
	sm::Matrix  m_rotationMatrix;

	float m_nearPlane;
	float m_farPlane;
	float m_windowHeight;
	float m_windowWidth;
	float m_aspectRatio;

	float m_rotationSpeed;
	float m_movingSepeed;

	camera_Matrix_t m_cameraMat;
	sm::Quaternion quaterion;

	/*Run in all the set functions*/
	void UpdateProjection();

public:
	Camera();
	~Camera();
	/* Position, Target, up, windowSize = (window width, window height) */
	void Initialize(sm::Vector3 pos, sm::Vector3 target, sm::Vector3 up, sm::Vector2 windowSize);
	void Update(float deltaTime);

	//Get Functions
	sm::Matrix GetView() const;
	sm::Matrix GetProjection() const;
	sm::Vector3 GetPosition() const;
	sm::Vector3 GetTarget() const;
	sm::Vector3 GetUp() const;
	camera_Matrix_t* GetCameraMatrixes();

	//Set Functions
	void SetFOV(float fov);
	void SetNearFarPlane(float nearPlane, float farPlane);
	void SetNearPlane(float nearPlane);
	void SetFarPlane(float farPlane);
	/*Val can only be between 0 and 1, 1 is normal*/
	void SetZoom(float val);

	//Public variables and stuff
	ComPtr<ID3D11Buffer> m_viewConstantBuffer;
	float m_FOV;
	float m_zoomValue;

};