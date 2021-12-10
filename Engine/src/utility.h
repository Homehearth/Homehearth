#pragma once
#include <SimpleMath.h>

namespace util {

	template<typename T>
	T Lerp(const T& a, const T& b, float t)
	{
		return a + t * (b - a);
	}

	sm::Vector3 Lerp(std::vector<sm::Vector3> points, float t);
	
	template<typename T>
	T BezierCurve(std::vector<T> points, float t)
	{
		t = min(t, 1.0f);
		return util::BezierCurve<T>(points.begin(), points.end(), t);
	}

	template<typename T>
	T BezierCurve(typename std::vector<T>::iterator begin, typename std::vector<T>::iterator end, float t)
	{
		if (begin == end)
			return *begin;

		for (auto it = begin; (it + 1) != end; it++)
		{
			*it = util::Lerp(*it, *(it + 1), t);
		}

		return util::BezierCurve<T>(begin, end - 1, t);
	}


	/*
		Rebases the old_value onto a new range between the new_max and new_min.
		Make sure that old_max and old_min are the max and min of old_value.
	*/
	template<typename T>
	T Rebase(const T& old_value, const T& old_max, const T& old_min, const T& new_max, const T& new_min)
	{
		T oldRange = old_max - old_min;
		T newRange = new_max - new_min;
		return (((old_value - old_min) * newRange) / oldRange) + new_min;
	}
	std::string VecToStr(const sm::Vector3& v);
	
}