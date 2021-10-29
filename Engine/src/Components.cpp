#include "EnginePCH.h"
#include "Components.h"
namespace ecs {

    sm::Matrix GetMatrix(const component::Transform& transform)
    {
        sm::Matrix mat = sm::Matrix::CreateScale(transform.scale);
        mat *= sm::Matrix::CreateWorld(transform.position, GetForward(transform), GetUp(transform));
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

    bool StepRotateTo(sm::Vector3& rotation, const sm::Vector3& target, float time)
    {
        float targetRotation = atan2(-target.z, target.x);

        float deltaRotation = targetRotation - rotation.y;

        if (std::abs(deltaRotation) > dx::g_XMPi[0])
        {
            rotation.y += (rotation.y < 0.0f) ? dx::g_XMTwoPi[0] : -dx::g_XMTwoPi[0];
        }
        rotation.y = rotation.y * (1 - time) + targetRotation * time;

        return std::abs(rotation.y - targetRotation) < 0.01f;
    }

    bool StepTranslateTo(sm::Vector3& translation, const sm::Vector3& target, float t)
    {
        translation = translation * (1 - t) + target * t;
        return sm::Vector3::Distance(translation, target) < 0.01f;
    }

}

