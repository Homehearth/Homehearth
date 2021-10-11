#include "GameSystems.h"

using namespace network;
void GameSystems::UserInputSystem(Scene& scene, Client& client)
{
	scene.ForEachComponent<comp::Transform, comp::Velocity, comp::Player>([&](comp::Transform&, comp::Velocity& velocity, comp::Player& player)
		{
			velocity.vel.z = InputSystem::Get().GetAxis(Axis::VERTICAL) * player.runSpeed;
			velocity.vel.x = InputSystem::Get().GetAxis(Axis::HORIZONTAL) * player.runSpeed;
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
			LOG_INFO("Mouseray HIT box detected!");
		}
	});
}

//Checks all OBB and BoundingSphere colliders in scene and IF collision push event.
void GameSystems::CollisionCheckSystem(Scene& scene)
{
	auto viewObb = scene.GetRegistry()->view<comp::BoundingOrientedBox>();
	auto viewSphere = scene.GetRegistry()->view<comp::BoundingSphere>();

	//Goes through all OBB and check collisions against spheres and other obb
	for (auto entity1 = viewObb.begin(), end = viewObb.end(); entity1 != end; ++entity1)
	{
		comp::BoundingOrientedBox obb1 = viewObb.get<comp::BoundingOrientedBox>(*entity1);


		//Collision check against comp::BoundingOrientedBox
		for (auto entity2 = entity1; entity2 != end; ++entity2)
		{
			comp::BoundingOrientedBox obb2 = viewObb.get<comp::BoundingOrientedBox>(*entity2);

			if (obb1.Intersects(obb2))
				scene.publish<ESceneCollision>(*entity1, *entity2);
				//If no collision check if they are pair in collisionSystem and remove them.
			else if (CollisionSystem::Get().IsColliding(*entity1, *entity2))
				CollisionSystem::Get().RemovePair(*entity1, *entity2);
		}

		//Collision check against comp::BoundingSphere
		for (auto entity2 = viewSphere.begin(); entity2 != viewSphere.end(); ++entity2)
		{
			comp::BoundingSphere sphere2 = viewSphere.get<comp::BoundingSphere>(*entity2);

			if (obb1.Intersects(sphere2))
				scene.publish<ESceneCollision>(*entity1, *entity2);
				//If no collision check if they are pair in collisionSystem and remove them.
			else if (CollisionSystem::Get().IsColliding(*entity1, *entity2))
				CollisionSystem::Get().RemovePair(*entity1, *entity2);
		}
	}

	//Goes through all BoundingSphere and check collision against other BoundingSphere
	for (auto entity1 = viewSphere.begin(), end = viewSphere.end(); entity1 != end; ++entity1)
	{
		comp::BoundingSphere sphere1 = viewSphere.get<comp::BoundingSphere>(*entity1);

		for (auto entity2 = entity1; entity2 != end; ++entity2)
		{
			comp::BoundingSphere sphere2 = viewSphere.get<comp::BoundingSphere>(*entity2);

			if (sphere1.Intersects(sphere2))
				scene.publish<ESceneCollision>(*entity1, *entity2);

				//If no collision check if they are pair in collisionSystem and remove them.
			else if (CollisionSystem::Get().IsColliding(*entity1, *entity2))
				CollisionSystem::Get().RemovePair(*entity1, *entity2);
		}
	}
}