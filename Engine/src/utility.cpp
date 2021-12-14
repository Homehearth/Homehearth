#include "EnginePCH.h"

sm::Vector3 util::Lerp(std::vector<sm::Vector3> points, float t)
{
	if (points.size() == 1)
		return points.front();

	if (points.size() == 0)
		return sm::Vector3(0, 0, 0);

	t = t * (points.size() - 1);
	int line = (int)t;
	return util::Lerp(points[line], points[line + 1], t);
}

sm::Vector2 util::WorldSpaceToScreenSpace(const sm::Vector3& worldPos, Camera* currentCam)
{
	sm::Vector4 oldP = worldPos;
	oldP.w = 1.0f;
	sm::Vector4 newP = dx::XMVector4Transform(oldP, currentCam->GetCameraMatrixes()->view);
	newP = dx::XMVector4Transform(newP, currentCam->GetCameraMatrixes()->projection);
	newP.x /= newP.w;
	newP.y /= newP.w;
	newP.z /= newP.w;

	// Conversion from NDC space [-1, 1] to Window space
	float new_x = (((newP.x + 1) * (D2D1Core::GetWindow()->GetWidth())) / (2));
	float new_y = D2D1Core::GetWindow()->GetHeight() - (((newP.y + 1) * (D2D1Core::GetWindow()->GetHeight())) / (2));

	return { new_x, new_y };
}

std::string util::VecToStr(const sm::Vector3& v)
{
	return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
}
