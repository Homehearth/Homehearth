#include "EnginePCH.h"
#include "Components.h"
namespace ecs {

    sm::Matrix GetMatrix(const component::Transform& transform)
    {
        sm::Matrix mat = sm::Matrix::CreateWorld(transform.position, GetForward(transform), GetUp(transform));
        mat *= sm::Matrix::CreateScale(transform.scale);
        return mat;
    }

    sm::Vector3 GetForward(const component::Transform& transform)
    {
        sm::Vector3 f = sm::Vector3::Forward;
        f = sm::Vector3::TransformNormal(f, sm::Matrix::CreateRotationX(transform.rotation.x));
        f = sm::Vector3::TransformNormal(f, sm::Matrix::CreateRotationY(transform.rotation.y));
        f = sm::Vector3::TransformNormal(f, sm::Matrix::CreateRotationZ(transform.rotation.z));
        return f;
    }

    sm::Vector3 GetUp(const component::Transform& transform) 
    {
        sm::Vector3 u = sm::Vector3::Up;
        u = sm::Vector3::TransformNormal(u, sm::Matrix::CreateRotationX(transform.rotation.x));  
        u = sm::Vector3::TransformNormal(u, sm::Matrix::CreateRotationY(transform.rotation.y));
        u = sm::Vector3::TransformNormal(u, sm::Matrix::CreateRotationZ(transform.rotation.z));
        return u;
    }
}