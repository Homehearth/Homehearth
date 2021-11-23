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

void Picture::Draw()
{
	if (m_border)
		m_border->Draw();
	if (m_texture[1])
		D2D1Core::DrawP(m_drawOpts, m_texture[1]->GetTexture());

	m_texture.ReadyForSwap();
}

void rtd::Picture::OnClick()
{

}

void rtd::Picture::OnHover()
{
}

ElementState rtd::Picture::CheckClick()
{
	return ElementState::NONE;
}

bool rtd::Picture::CheckHover()
{
	return false;
}

