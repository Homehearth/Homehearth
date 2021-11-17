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
        
        float targetRotation = atan2(-targetVector.x, -targetVector.z);
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

    bool UseAbility(component::IAbility* abilityComponent, sm::Vector3* targetPoint)
    {
        if (targetPoint)
            abilityComponent->targetPoint = *targetPoint;
        
        if (abilityComponent->isReady)
        {
            abilityComponent->isUsing = true;
            abilityComponent->isReady = false;
            abilityComponent->cooldownTimer = abilityComponent->cooldown;
            abilityComponent->delayTimer = abilityComponent->delay;
            abilityComponent->useTimer = abilityComponent->useTime;
        }
        return abilityComponent->isUsing;
    }
    
    bool UseAbility(Entity entity, entt::meta_type abilityType, sm::Vector3* targetPoint)
    {
        comp::IAbility* ability = GetAbility(entity, abilityType);
        if (!ability)
        {
            LOG_WARNING("This entity does not have this ability");
            return false;
        }
        return UseAbility(ability, targetPoint);
    }

    bool ReadyToUse(component::IAbility* abilityComponent, sm::Vector3* targetPoint)
    {
        if (targetPoint)
            abilityComponent->targetPoint = *targetPoint;

        if (abilityComponent->isUsing && abilityComponent->delayTimer <= 0.f)
        {
            abilityComponent->cooldownTimer = abilityComponent->cooldown;
            abilityComponent->isUsing = false;
            return true;
        }
        return false;
    }

    bool ReadyToUse(Entity entity, entt::meta_type abilityType, sm::Vector3* targetPoint)
    {
        comp::IAbility* ability = GetAbility(entity, abilityType);
        if (!ability)
        {
            LOG_WARNING("This entity does not have this ability");
            return false;
        }
        return ReadyToUse(ability, targetPoint);

    }

    bool IsUsing(const component::IAbility* abilityComponent)
    {
        return abilityComponent->isUsing || abilityComponent->useTimer > 0.f;
    }


    bool IsUsing(Entity entity, entt::meta_type abilityType)
    {
        comp::IAbility* ability = GetAbility(entity, abilityType);
        if (!ability)
        {
            LOG_WARNING("This entity does not have this ability");
            return false;
        }
        return IsUsing(ability);
    }

    bool IsPlayerUsingAnyAbility(Entity player)
    {
        component::Player* p = player.GetComponent<component::Player>();
        if (p)
        {
            return IsUsing(player, p->primaryAbilty) || IsUsing(player, p->secondaryAbilty);
        }
        return false;
    }

    component::IAbility* GetAbility(Entity entity, entt::meta_type abilityType)
    {
        using namespace entt::literals;

        auto func = abilityType.func("get"_hs);
        if (!func)
        {
            return nullptr;
        }
        auto instance = func.invoke({}, entity);
        component::IAbility* ability = instance.try_cast<component::IAbility>();

        return ability;
    }

    component::TemporaryPhysics::Force GetGravityForce()
    {
        component::TemporaryPhysics::Force f = {};
        f.force = sm::Vector3(0, -50.f, 0);
        f.isImpulse = false;
        return f;
    }
}

