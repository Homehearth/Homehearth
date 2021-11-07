#pragma once
#include "EnginePCH.h"
#include "Components.h"

#define CB_CAM_SLOT 1

enum class CAMERATYPE
{
	DEFAULT,
	DEBUG,
	PLAY
};

class Camera
{
private:
	sm::Vector3 m_up;
	sm::Vector3 m_target;
	sm::Vector3 m_forward;
	sm::Vector3 m_right;
	sm::Vector3 m_defaultForward;
	sm::Vector3 m_defaultRight;
	sm::Vector3 m_defaultPos;
	sm::Vector3 m_move;

	Entity m_targetEntity;

	sm::Matrix  m_view;
	sm::Matrix  m_projection;
	sm::Matrix  m_rotationMatrix;

	float m_windowHeight;
	float m_windowWidth;
	float m_aspectRatio;

	float m_rotationSpeed;
	float m_movingSpeed;

	sm::Quaternion quaterion;
	CAMERATYPE m_type;

	/*Run in all the set functions*/
	void UpdateProjection();


	camera_Matrix_t m_cameraMat;
	ComPtr<ID3D11Buffer> m_CameraConstantBuffer;
	void UpdateConstantBuffer();

public:
	Camera();
	~Camera();
	/* Position, Target, up, windowSize = (window width, window height) */
	void Initialize(sm::Vector3 pos, sm::Vector3 target, sm::Vector3 up, sm::Vector2 windowSize, CAMERATYPE type) ;
	void Update();
	void SetFollowEntity(const Entity& entity);

	//Bind the camera to shader
	void BindCB();
	void UnbindCB();

	//Get Functions
	sm::Matrix GetView() const;
	sm::Matrix GetProjection() const;
	sm::Vector3 GetPosition() const;
	sm::Vector3 GetTarget() const;
	sm::Vector3 GetUp() const;
	camera_Matrix_t* GetCameraMatrixes();
	CAMERATYPE GetCameraType()const;
	sm::Vector3 GetRollPitchYaw() const;

	//Set Functions
	void SetPosition(sm::Vector3 newPosition);
	void SetFOV(float fov);
	void SetNearFarPlane(float nearPlane, float farPlane);
	void SetNearPlane(float nearPlane);
	void SetFarPlane(float farPlane);
	/*Val can only be between 0 and 1, 1 is normal*/
	void SetZoom(float val);
	void SetRollPitchYaw(sm::Vector3 rotation);

	float m_FOV;
	float m_zoomValue;
	float m_nearPlane;
	float m_farPlane;
	sm::Vector3 m_rollPitchYaw;
	sm::Quaternion m_rotation = sm::Quaternion::Identity;
	sm::Vector3 m_position;
};


// component version
namespace ecs::component {
	struct Camera3D {
		Camera camera;
	};
}