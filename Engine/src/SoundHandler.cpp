#include "EnginePCH.h"
#include "SoundHandler.h"

SoundHandler::SoundHandler()
{
    // Start the sound engine with default parameters.
    m_engine = irrklang::createIrrKlangDevice();

    if (m_engine == nullptr)
    {
        LOG_ERROR("failed starting up the irrklang sound engine.")
    }
    else
    {
        m_engine->setDefault3DSoundMinDistance(40.0f);
        m_engine->setRolloffFactor(70.0f);
        m_engine->setSoundVolume(0.5f);

        LOG_INFO("Irrklang sound engine successfully loaded.")
    }

    // Create the sound object
    m_ss = m_engine->addSoundSourceFromFile("../Assets/Sounds/on_click.wav");
    m_ss2 = m_engine->addSoundSourceFromFile("../Assets/Sounds/main_theme.mp3");
    PlayMainMenuTheme();
}

SoundHandler::~SoundHandler()
{
    if(m_engine != nullptr)
    {
        m_engine->stopAllSounds();
        m_engine->removeAllSoundSources();
        m_engine->drop();
    }
}

SoundHandler& SoundHandler::Get()
{
    static SoundHandler s_instance;
    return s_instance;
}

void SoundHandler::AddSound(const std::string &filePath)
{
	// parse filename.
    // check if it already excists.
    // load and save it to map.
    // LOG_INFO
}

void SoundHandler::PlayOnClick()
{
    if (m_engine->isCurrentlyPlaying(m_ss))
    {
        LOG_INFO("on_click.wav is playing")
    }
    else
        m_engine->play2D(m_ss);
}

void SoundHandler::PlayMainMenuTheme()
{
    if (m_engine->isCurrentlyPlaying(m_ss2))
    {
        LOG_INFO("main_theme is playing")
    }
    else
        m_engine->play2D(m_ss2);
}

