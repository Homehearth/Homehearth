#include "EnginePCH.h"
#include "Picture.h"

using namespace rtd;

Picture::Picture(const std::string& fileName, const draw_t& opts)
{
	m_texture = ResourceManager::Get().GetResource<RBitMap>(fileName);
	m_drawOpts = opts;
	m_border = nullptr;
	this->SetLayer(m_drawOpts.layer);
}

rtd::Picture::~Picture()
{
	if (m_border)
	{
		delete m_border;
	}
}

Border* rtd::Picture::GetBorder()
{
	if (!m_border)
	{
		m_border = new Border(m_drawOpts);
	}
	return m_border;
}


void rtd::Picture::RemoveBorder()
{
	if (m_border)
	{
		delete m_border;
	}

}

void Picture::Draw()
{
	if (m_border)
		m_border->Draw();
	if(m_texture)
		D2D1Core::DrawP(m_drawOpts, m_texture->GetTexture());
}

const bool rtd::Picture::IsClicked()
{
	// Check if mouse key is pressed.
	if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
	{
		// Is within bounds?
		if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos &&
			InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width &&
			InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos &&
			InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height)
		{
			return true;
		}
	}
	return false;
}

void rtd::Picture::OnClick()
{
	m_drawOpts.x_pos = rand() % 400;
	m_drawOpts.y_pos = rand() % 400;
	if (m_border)
		m_border->UpdatePos(m_drawOpts);
}

