#include "EnginePCH.h"
#include "AbilityUI.h"

void rtd::AbilityUI::Update()
{
    if (m_cooldown <= 0)
    {
        m_cooldownText->SetText(m_buttonPress);
    }
    else
        m_cooldownText->SetText(std::to_string((int)m_cooldown));
}

rtd::AbilityUI::AbilityUI(const draw_t& opts, const D2D1_COLOR_F& color, const std::string& picturePath)
{
    m_border = std::make_unique<Border>(opts);
    m_picture = std::make_unique<Picture>(picturePath, draw_t(opts.x_pos, opts.y_pos, opts.width, opts.height));
    m_cooldownText = std::make_unique<Text>("Unk", opts);
    m_buttonPress = "Unk";
}

void rtd::AbilityUI::SetPicture(const std::string& filePath)
{
    m_picture->SetTexture(filePath);
}

void rtd::AbilityUI::SetActivateButton(const std::string& button)
{
    m_buttonPress = button;
}

void rtd::AbilityUI::SetRoundBorder(const float& radius)
{
    m_border->SetRound(radius);
}

void rtd::AbilityUI::Draw()
{
    //if (m_border)
    //    m_border->Draw();
    if (m_picture)
        m_picture->Draw();
    if (m_cooldownText)
        m_cooldownText->Draw();
}

void rtd::AbilityUI::OnClick()
{
    /*
        If player presses the ability UI should it activate maybe?
    */
}

void rtd::AbilityUI::OnHover()
{
    this->Update();
}

bool rtd::AbilityUI::CheckHover()
{
    return true;
}

bool rtd::AbilityUI::CheckClick()
{
    return false;
}
