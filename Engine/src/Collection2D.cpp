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
	/*
		If any of the elements were pressed inside we act 
	*/
	ElementState checkSum = ElementState::NONE;
	for (size_t i = 0; i < elements.size(); i++)
	{
		if (elements[i]->IsVisible())
		{
			if (elements[i]->CheckClick() == ElementState::INSIDE)
			{
				elements[i]->OnClick();
				checkSum = ElementState::INSIDE;
			}
			else
			{
				if (checkSum != ElementState::INSIDE)
				{
					checkSum = ElementState::OUTSIDE;
				}
			}
			if (elements[i]->CheckHover())
			{
				elements[i]->OnHover();
			}
		}
	}

	m_state = checkSum;
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

const ElementState& Collection2D::GetState() const
{
	return m_state;
}
