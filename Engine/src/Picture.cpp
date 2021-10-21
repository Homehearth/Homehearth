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
	m_texture.reset();
	m_texture = ResourceManager::Get().GetResource<RBitMap>(fileName);
}

void rtd::Picture::UpdatePos(const draw_t& new_pos)
{
	m_drawOpts = new_pos;
}

void Picture::Draw()
{
	if (m_border)
		m_border->Draw();
	if(m_texture)
		D2D1Core::DrawP(m_drawOpts, m_texture->GetTexture());
}

void rtd::Picture::OnClick()
{

}

void rtd::Picture::OnHover()
{
}

const bool rtd::Picture::Update()
{
	return false;
}

const bool rtd::Picture::CheckHover()
{
	return false;
}

