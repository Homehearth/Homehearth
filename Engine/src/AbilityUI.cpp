#include "EnginePCH.h"
#include "AbilityUI.h"

void rtd::AbilityUI::Update()
{
    if (m_ref)
        m_cooldown = *m_ref;

    if (m_cooldown <= 0)
    {
        m_cooldownText->SetText(m_buttonPress);
    }
    else
    {
        std::string tmp = std::to_string(m_cooldown);
        std::string value;
        auto length = std::snprintf(&value[0], value.size(), "%.1f", m_cooldown);
        tmp.resize(length);
        m_cooldownText->SetText(tmp);
    }
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

void rtd::AbilityUI::SetReference(float* ref)
{
    m_ref = ref;
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

ElementState rtd::AbilityUI::CheckClick()
{
    return ElementState::NONE;
}
