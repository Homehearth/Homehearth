#include "GameSystems.h"
#include "EnginePCH.h"
#include "Healthbar.h"

using namespace network;


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

//System to render collider mesh red if collider is colliding with another collider
void GameSystems::RenderIsCollidingSystem(Scene& scene)
{
	scene.ForEachComponent<comp::RenderableDebug>([&](Entity entity, comp::RenderableDebug& renderableDebug)
		{
			//Collided with mouse TODO make it do someting?
			if(CollisionSystem::Get().getCollisionCounts(entity) > 0)
			{
				renderableDebug.isColliding.hit = 1;
			}
			else
			{
				renderableDebug.isColliding.hit = 0;
			}
		});
}

// Set all the healthbars to players.
void GameSystems::UpdateHealthbar(Scene& scene)
{
	int i = 1;
	scene.ForEachComponent<comp::Health, comp::Player>([&](Entity e,comp::Health& health, const comp::Player& player) {
		// Safety check for now.
		if (i < 5)
		{
			rtd::Healthbar* healthbar = dynamic_cast<rtd::Healthbar*>(scene.GetCollection("player" + std::to_string(i) + "Info")->elements[0].get());
			if (healthbar)
			{
				healthbar->SetHealthVariable(e);
			}
		}
		i++;
		});
}

void GameSystems::UpdateNamePlate(Scene& scene)
{
	int i = 1;
	scene.ForEachComponent<comp::NamePlate, comp::Transform>([&](Entity& e, comp::NamePlate& name, comp::Transform &t)
		{
			if (i < 5)
			{
				Collection2D* collection = scene.GetCollection("dynamicPlayer" + std::to_string(i) + "namePlate");
				if (collection)
				{
					rtd::Text* namePlate = dynamic_cast<rtd::Text*>(collection->elements[0].get());
					if (namePlate)
					{
						Camera* cam = scene.GetCurrentCamera();

						sm::Vector4 oldP = { t.position.x, t.position.y + 20.0f, t.position.z, 1.0f };
						sm::Vector4 newP = dx::XMVector4Transform(oldP, cam->GetCameraMatrixes()->view);
						newP = dx::XMVector4Transform(newP, cam->GetCameraMatrixes()->projection);
						newP.x /= newP.w;
						newP.y /= newP.w;
						newP.z /= newP.w;

						// Conversion from NDC space [-1, 1] to Window space [960, 540]
						float new_x = (((newP.x + 1) * (D2D1Core::GetWindow()->GetWidth())) / (2));
						float new_y = D2D1Core::GetWindow()->GetHeight() - (((newP.y + 1) * (D2D1Core::GetWindow()->GetHeight())) / (2));

						namePlate->SetPosition(new_x - ((name.namePlate.length() * D2D1Core::GetDefaultFontSize()) * 0.25f), new_y);
						namePlate->SetText(name.namePlate);

						namePlate->SetVisiblity(true);
					}
				}
			}
			i++;
		});
}