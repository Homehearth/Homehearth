#include "GameSystems.h"

using namespace network;
void GameSystems::UserInputSystem(Scene& scene, Client& client)
{
	scene.ForEachComponent<comp::Transform, comp::Velocity, comp::Player>([&](comp::Transform&, comp::Velocity& velocity, comp::Player& player)
		{
			velocity.vel.z = -InputSystem::Get().GetAxis(Axis::VERTICAL) * player.runSpeed;
			velocity.vel.x = -InputSystem::Get().GetAxis(Axis::HORIZONTAL) * player.runSpeed;
		});
}

//System check if mouse ray intersects any of the box collider components in scene
void GameSystems::MRayIntersectBoxSystem(Scene& scene)
{
	float t = 0;
	
	scene.ForEachComponent<comp::BoundingOrientedBox, comp::Transform>([&](Entity entity, comp::BoundingOrientedBox& boxCollider, comp::Transform& transform)
    {
		//Collided with mouse TODO make it do someting?
		if(Intersect::RayIntersectBox(InputSystem::Get().GetMouseRay(), boxCollider, t))
		{
			LOG_INFO("Mouse position %f, %f, %f", InputSystem::Get().GetMouseRay().rayPos.x, InputSystem::Get().GetMouseRay().rayPos.y, InputSystem::Get().GetMouseRay().rayPos.z);
			LOG_INFO("Transform position %f, %f, %f", transform.position.x, transform.position.y, transform.position.z);
			//transform.scale = sm::Vector3(1.5f, 1.5f, 1.5f);
		}
		else
			transform.scale = sm::Vector3(1.f, 1.f, 1.f);
		
	});
}

void GameSystems::CollisionSystem(Scene& scene)
{
	scene.ForEachComponent<comp::Transform>([&](Entity entity, comp::Transform& transform)
		{
			comp::BoundingOrientedBox* obb1 = entity.GetComponent<comp::BoundingOrientedBox>();
			comp::BoundingSphere* sphere1 = entity.GetComponent<comp::BoundingSphere>();

			if(obb1 != nullptr)
			{
				scene.ForEachComponent<comp::Transform>([&](Entity entity2, comp::Transform& transform)
				{
					if(entity != entity2)
					{
						comp::BoundingOrientedBox* obb2 = entity2.GetComponent<comp::BoundingOrientedBox>();
						comp::BoundingSphere* sphere2 = entity2.GetComponent<comp::BoundingSphere>();
						
						if(obb1 && obb2 && obb1->Intersects(*obb2))
						{
							scene.publish<ESceneCollision>(entity, entity2);
						}
					}
				});

			}
			else if(sphere1 != nullptr)
			{
				scene.ForEachComponent<comp::Transform>([&](Entity entity2, comp::Transform& transform)
					{
						if (entity != entity2)
						{
							comp::BoundingOrientedBox* obb2 = entity2.GetComponent<comp::BoundingOrientedBox>();
							comp::BoundingSphere* sphere2 = entity2.GetComponent<comp::BoundingSphere>();
							
							if (sphere1 && obb2 && sphere1->Intersects(*obb2))
							{
								scene.publish<ESceneCollision>(entity, entity2);
							}
						}
					});
			}
		});







	
	//const auto viewOBB = scene.GetRegistry().view<comp::BoundingOrientedBox>();
	//const auto viewSphere = scene.GetRegistry().view<comp::BoundingSphere>();
	//for(auto entity: viewOBB)
	//{
	//	auto& obb1 = viewOBB.get<comp::BoundingOrientedBox>(entity);
	//	for(auto entity2: viewOBB)
	//	{
	//		if(entity != entity2)
	//		{
	//			auto& obb2 = viewOBB.get<comp::BoundingOrientedBox>(entity2);
	//			if(obb1.Intersects(obb2))
	//			{
	//				scene.publish<ESceneCollision>(entity, entity2);
	//			}
	//		}
	//	}
	//}
}


