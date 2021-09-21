#pragma once
#include "RMesh.h"
#include "RTexture.h"

namespace ecs
{
	namespace component {
		struct Transform
		{
			sm::Vector3 position;
			sm::Vector3 rotation;
			sm::Vector3 scale;

			std::unique_ptr<dx::ConstantBuffer<sm::Matrix>> constBuf;
			//ID3D11Buffer* constBuf = nullptr;
		};

		struct RenderAble
		{
			RMesh* mesh = nullptr;
			RTexture* texture = nullptr;

			// Constant buffer
			std::shared_ptr<dx::ConstantBuffer<sm::Matrix>> constBuf;
			//ID3D11Buffer* constBuf = nullptr;
		};

		struct Mesh
		{
			RMesh* mesh;
		};
	};

	void OnTransformConstruct(entt::registry& reg, entt::entity entity);
	void OnTransformUpdate(entt::registry& reg, entt::entity entity);
	void OnRenderAbleConstruct(entt::registry& reg, entt::entity entity);

	sm::Matrix GetMatrix(component::Transform& transform);
	sm::Vector3 GetForward(component::Transform& transform);
	sm::Vector3 GetUp(component::Transform& transform);

};

namespace comp = ecs::component;
