#pragma once

class GuiElement : public entt::emitter<GuiElement>
{
private:
	std::vector<std::unique_ptr<GuiElement>> m_children;
protected:
	bool m_isHovering;
	RECT m_contentRect;

	void DrawChildren();
public:
	
	template<typename T, typename ...Args>
	T* addChild(Args... args);

	bool IsHovering() const;
	
	virtual void Draw() = 0;
}; 

template<typename T, typename ...Args>
inline T* GuiElement::addChild(Args... args)
{
	children.emplace_back(std::make_unique<T>(args...));
	return dynamic_cast<T*>(children.back().get());
}
