#include "EnginePCH.h"
#include "Picture.h"

using namespace rtd;

Picture::Picture(const std::string& fileName, const draw_t& opts)
{
	m_texture[0] = ResourceManager::Get().GetResource<RBitMap>(fileName);
	m_texture[1] = ResourceManager::Get().GetResource<RBitMap>(fileName);
	m_drawOpts = opts;
	m_border = nullptr;
}

rtd::Picture::Picture()
{
	m_border = nullptr;
}

rtd::Picture::~Picture()
{
}

Border* rtd::Picture::GetBorder()
{
	if (!m_border)
	{
		m_border = std::make_unique<Border>(m_drawOpts);
	}
	return m_border.get();
}


void rtd::Picture::RemoveBorder()
{
	if (m_border)
	{
		delete m_border.get();
	}
}

void rtd::Picture::SetTexture(const std::string& fileName)
{
	m_texture[0].reset();
	m_texture[0] = ResourceManager::Get().GetResource<RBitMap>(fileName);

	if (!m_texture.IsSwapped())
		m_texture.Swap();
}

void rtd::Picture::UpdatePos(const draw_t& new_pos)
{
	m_drawOpts = new_pos;
}

void rtd::Picture::SetPosition(const FLOAT& x, const FLOAT& y)
{
	m_drawOpts.x_pos = x;
	m_drawOpts.y_pos = y;
}

void rtd::Picture::SetOpacity(const FLOAT& opacity)
{
	m_opacity = opacity;
}

void rtd::Picture::SetOnHoverEvent(const std::function<void()>& func)
{
	m_hoverFunction = func;
}

void Picture::Draw()
{
	if (m_border)
		m_border->Draw();
	if (m_texture[1])
		D2D1Core::DrawP(m_drawOpts, m_texture[1]->GetTexture(), m_opacity);

	m_texture.ReadyForSwap();
}

void rtd::Picture::OnClick()
{

}

void rtd::Picture::OnHover()
{
	if (m_hoverFunction)
	{
		m_hoverFunction();
	}
}

ElementState rtd::Picture::CheckClick()
{
	if (CheckHover())
	{
		// CheckCollisions if mouse key is pressed.
		if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
		{
			m_isClicked = true;
			return ElementState::INSIDE;
		}
	}
	else
	{
		// CheckCollisions if mouse key is pressed.
		if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
		{
			return ElementState::OUTSIDE;
		}
	}

	return ElementState::NONE;
}

bool rtd::Picture::CheckHover()
{
	// Is within bounds?
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height)
	{
		return true;
	}
	return false;
}

