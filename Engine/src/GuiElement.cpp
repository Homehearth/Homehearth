#include "EnginePCH.h"
#include "GuiElement.h"

void GuiElement::DrawChildren() {
	for (auto& child : m_children) {
		child->Draw();
	}
}

bool GuiElement::IsHovering() const
{
	return m_isHovering;
}
