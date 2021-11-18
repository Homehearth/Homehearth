#pragma once
#include <SimpleMath.h>

namespace util {

	template<typename T>
	T Lerp(const T& a, const T& b, float t)
	{
		return a + t * (b - a);
	}
	sm::Vector3 Lerp(std::vector<sm::Vector3> points, float t);
	
	sm::Vector3 BezierCurve(std::vector<sm::Vector3> points, float t);
	sm::Vector3 BezierCurve(std::vector<sm::Vector3>::iterator begin, std::vector<sm::Vector3>::iterator end, float t);
	
	
}
