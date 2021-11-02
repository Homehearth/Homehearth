#include "EnginePCH.h"
#include "Collection2D.h"

Collection2D::Collection2D()
{
}

Collection2D::~Collection2D()
{
}

void Collection2D::UpdateCollection()
{
	for (size_t i = 0; i < elements.size(); i++)
	{
		if (elements[i]->CheckClick())
		{
			elements[i]->OnClick();
		}
		if (elements[i]->CheckHover())
		{
			elements[i]->OnHover();
		}
	}
}

void Collection2D::Show()
{
	for (size_t i = 0; i < elements.size(); i++)
	{
		elements[i]->SetVisiblity(true);
	}
}

void Collection2D::Hide()
{
	for (size_t i = 0; i < elements.size(); i++)
	{
		elements[i]->SetVisiblity(false);
	}
}