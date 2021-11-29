#pragma once

enum class CAMERATYPE
{
	DEFAULT,
	DEBUG,
	PLAY
};

static DoubleBuffer<camera_Matrix_t> s_cameraBuffers;

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

	camera_Matrix_t m_cameraMat;
	sm::Quaternion quaterion;
	CAMERATYPE m_type;

	/*Run in all the set functions*/
	void UpdateProjection();

public:
	Camera();
	~Camera();
	/* Position, Target, up, windowSize = (window width, window height) */
	void Initialize(sm::Vector3 pos, sm::Vector3 target, sm::Vector3 up, sm::Vector2 windowSize, CAMERATYPE type) ;
	void Update(const float& deltaTime);
	void SetFollowEntity(const Entity& entity);


	//Get Functions
	sm::Matrix GetView() const;
	sm::Matrix GetProjection() const;
	sm::Vector3 GetPosition() const;
	sm::Vector3 GetTarget() const;
	sm::Vector3 GetUp() const;
	camera_Matrix_t* GetCameraMatrixes();
	void Swap();
	void ReadySwap();
	bool IsSwapped() const;
	CAMERATYPE GetCameraType()const;
	sm::Vector3 GetRollPitchYaw() const;
	sm::Quaternion GetRotationMatrix() const;

	//Set Functions
	void SetPosition(sm::Vector3 newPosition);
	void SetFOV(float fov);
	void SetNearFarPlane(float nearPlane, float farPlane);
	void SetNearPlane(float nearPlane);
	void SetFarPlane(float farPlane);
	/*Val can only be between 0 and 1, 1 is normal*/
	void SetZoom(float val);
	void SetRollPitchYaw(sm::Vector3 rotation);

	//Public variables and stuff for IMGU
	ComPtr<ID3D11Buffer> m_viewConstantBuffer;

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