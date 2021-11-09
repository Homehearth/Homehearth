#pragma once
#include "Element2D.h"

class Collection2D
{
private:

public:
	Collection2D();
	virtual ~Collection2D();

	void UpdateCollection();
	void Show();
	void Hide();

	std::vector<std::unique_ptr<Element2D>> elements;

	template<typename T, typename ...Args>
	T* AddElement(Args... args);
};

template<typename T, typename ...Args>
inline T* Collection2D::AddElement(Args... args)
{
	elements.emplace_back(std::make_unique<T>(args...));

	return dynamic_cast<T*>(elements.back().get());
}