#include "EnginePCH.h"
#include "Components.h"

void ecs::OnTransformConstruct(entt::registry& reg, entt::entity entity)
{
    component::Transform& transform = reg.get<component::Transform>(entity);
    transform.pConstantBuffer = std::make_unique<dx::ConstantBuffer<sm::Matrix>>();
    transform.pConstantBuffer->Create(D3D11Core::Get().Device());

    sm::Matrix m = sm::Matrix::Identity;
    T_LOCK();
    transform.pConstantBuffer->SetData(D3D11Core::Get().DeviceContext(), m);
    T_UNLOCK();
}

void ecs::OnTransformUpdate(entt::registry& reg, entt::entity entity) {
    component::Transform& transform = reg.get<component::Transform>(entity);

    sm::Matrix m = GetMatrix(transform);
    T_LOCK();
    transform.pConstantBuffer->SetData(D3D11Core::Get().DeviceContext(), m);
    T_UNLOCK();
}

void ecs::OnRenderableConstruct(entt::registry& reg, entt::entity entity) {
    component::Renderable& renderable = reg.get<component::Renderable>(entity);
    renderable.pConstantBuffer = std::make_unique<dx::ConstantBuffer<sm::Matrix>>();
    renderable.pConstantBuffer->Create(D3D11Core::Get().Device());

    sm::Matrix m = sm::Matrix::Identity;
    T_LOCK();
    renderable.pConstantBuffer->SetData(D3D11Core::Get().DeviceContext(), m);
    T_UNLOCK();
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
