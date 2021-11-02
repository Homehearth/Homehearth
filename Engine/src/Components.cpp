#include "EnginePCH.h"
#include "Components.h"
namespace ecs {

    sm::Matrix GetMatrix(const component::Transform& transform)
    {
        sm::Matrix mat = sm::Matrix::CreateScale(transform.scale);
        mat *= sm::Matrix::CreateFromQuaternion(transform.rotation);
        mat.Translation(transform.position);
        return mat;
    }

    sm::Vector3 GetForward(const component::Transform& transform)
    {
        sm::Vector3 f = sm::Vector3::Forward;
        f = sm::Vector3::TransformNormal(f, sm::Matrix::CreateFromQuaternion(transform.rotation));
        return f;
    }

    sm::Vector3 GetRight(const component::Transform& transform) 
    {
        sm::Vector3 r = GetForward(transform).Cross(sm::Vector3::Up);
        return r;
    }

    bool StepRotateTo(sm::Quaternion& rotation, const sm::Vector3& targetVector, float t)
    {
        
        float targetRotation = atan2(-targetVector.z, targetVector.x);
        sm::Quaternion targetQuat = sm::Quaternion::CreateFromAxisAngle(sm::Vector3::Up, targetRotation);
        targetQuat.Normalize();
        if (t >= 1.0f)
        {
            rotation = targetQuat;
            return true;
        }
        rotation = sm::Quaternion::Slerp(rotation, targetQuat, t);
        rotation.Normalize();

        if (std::abs(rotation.Dot(targetQuat)) > 1 - 0.01f)
        {
            rotation = targetQuat;
            return true;
        }
        return false;
    }

    bool StepTranslateTo(sm::Vector3& translation, const sm::Vector3& target, float t)
    {
        translation = translation * (1 - t) + target * t;
        return sm::Vector3::Distance(translation, target) < 0.01f;
    }

}

