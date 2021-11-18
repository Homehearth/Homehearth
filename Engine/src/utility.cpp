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

sm::Vector3 util::BezierCurve(std::vector<sm::Vector3> points, float t)
{
	return util::BezierCurve(points.begin(), points.end(), t);
}

sm::Vector3 util::BezierCurve(std::vector<sm::Vector3>::iterator begin, std::vector<sm::Vector3>::iterator end, float t)
{
	if (begin == end)
		return *begin;

	for (auto it = begin; (it + 1) != end; it++)
	{	
		*it = util::Lerp(*it, *(it + 1), t);
	}

	return util::BezierCurve(begin, end - 1, t);
}