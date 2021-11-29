#include "EnginePCH.h"
#include "Engine.h"
#include "SpreeHandler.h"

SpreeHandler::SpreeHandler()
{

}

void SpreeHandler::Update()
{
    if (m_timer > 0.0f)
    {
        // Countdown the spree.
        m_timer -= Stats::Get().GetUpdateTime();
    }
    else
    {
        // Reset the spree.
        m_currentSpree = 1;
    }
}

const unsigned int& SpreeHandler::GetSpree() const
{
    return m_currentSpree;
}

void SpreeHandler::AddSpree()
{
    m_currentSpree++;
    m_timer = SPREE_TIME;
}
