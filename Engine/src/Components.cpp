#include "EnginePCH.h"
#include "Components.h"

void ecs::OnTransformConstruct(entt::registry& reg, entt::entity entity)
{
    component::Transform& transform = reg.get<component::Transform>(entity);
    /*
    transform.pConstantBuffer = ResourceManager::Insert("sdf", new dx::ConstantBuffer<PerFrame>)
    transform->Create(D3D11Core::Get().Device());
    */

    cbuffer::PerObject perObject;
    perObject.world = ecs::GetMatrix(reg.get<component::Transform>(entity));
    //transform.constantBuffer.SetData(D3D11Core::Get().DeviceContext(), perObject);
}


sm::Matrix ecs::GetMatrix(component::Transform& transform)
{
    sm::Matrix mat = sm::Matrix::CreateWorld(transform.position, GetForward(transform), GetUp(transform));
    mat *= sm::Matrix::CreateScale(transform.scale);
    return mat;
}

sm::Vector3 ecs::GetForward(component::Transform& transform)
{
    sm::Vector3 f = sm::Vector3::Forward;
    f = sm::Vector3::TransformNormal(f, sm::Matrix::CreateRotationX(transform.rotation.x));
    f = sm::Vector3::TransformNormal(f, sm::Matrix::CreateRotationY(transform.rotation.y));
    f = sm::Vector3::TransformNormal(f, sm::Matrix::CreateRotationZ(transform.rotation.z));
    return f;
}

sm::Vector3 ecs::GetUp(component::Transform& transform) {
    sm::Vector3 u = sm::Vector3::Up;
    u = sm::Vector3::TransformNormal(u, sm::Matrix::CreateRotationX(transform.rotation.x));
    u = sm::Vector3::TransformNormal(u, sm::Matrix::CreateRotationY(transform.rotation.y));
    u = sm::Vector3::TransformNormal(u, sm::Matrix::CreateRotationZ(transform.rotation.z));
    return u;
}
