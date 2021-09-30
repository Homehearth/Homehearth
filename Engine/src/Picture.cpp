#include "EnginePCH.h"
#include "Picture.h"

Picture::Picture(const std::string& fileName, const _DRAW& opts)
{

	m_texture = ResourceManager::GetResource<RBitMap>(fileName);
	m_drawOpts = opts;
	this->SetName(fileName);
	this->SetLayer(m_drawOpts.layer);
}

void Picture::Draw()
{
	if(m_texture)
		D2D1Core::DrawP(m_drawOpts, m_texture->GetTexture());
}
