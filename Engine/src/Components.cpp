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

}

network::message<GameMsg>& operator << (message<GameMsg>& msg, const ecs::component::Transform& data)
{
    msg << data.position.x << data.position.y << data.position.z;
    msg << data.rotation.x << data.rotation.y << data.rotation.z;
    msg << data.scale.x << data.scale.y << data.scale.z;
    return msg;
}

network::message<GameMsg>& operator >> (message<GameMsg>& msg, ecs::component::Transform& data)
{
    msg >> data.scale.z >> data.scale.y >> data.scale.x;
    msg >> data.rotation.z >> data.rotation.y >> data.rotation.x;
    msg >> data.position.z >> data.position.y >> data.position.x;
    return msg;
}