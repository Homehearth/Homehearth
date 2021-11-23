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