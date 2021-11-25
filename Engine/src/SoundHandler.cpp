#include "EnginePCH.h"
#include "SoundHandler.h"

SoundHandler::SoundHandler()
{
    // Start the sound engine with default parameters.
    m_soundEngine = irrklang::createIrrKlangDevice();

    if (m_soundEngine == nullptr)
    {
        LOG_ERROR("failed starting up the irrklang sound engine.")
    }
    else
    {
        m_soundEngine->setDefault3DSoundMinDistance(40.0f);
        m_soundEngine->setRolloffFactor(70.0f);
        m_soundEngine->setSoundVolume(0.5f);

        LOG_INFO("Irrklang sound engine successfully loaded.")
    }

    // TEST
    AddSoundSource("../Assets/Sounds/on_click.wav");
    AddSoundSource("../Assets/Sounds/main_theme.mp3", "main_theme");

}

SoundHandler::~SoundHandler()
{
    if(m_soundEngine != nullptr)
    {
        m_soundEngine->stopAllSounds();
        m_soundEngine->removeAllSoundSources();
        m_soundEngine->drop();
    }
}

SoundHandler& SoundHandler::Get()
{
    static SoundHandler s_instance;
    return s_instance;
}

void SoundHandler::Update()
{
    m_soundEngine->update();
}

void SoundHandler::AddSoundSource(const std::string &filePath)
{
    // Parse filename.
    std::string sourceName = filePath.substr(filePath.find_last_of('/') + 1);
    const size_t nameLength = sourceName.find_last_of('.');
    sourceName = sourceName.substr(0, nameLength);

    AddSoundSource(filePath, sourceName);
}

void SoundHandler::AddSoundSource(const std::string& filePath, const std::string& name)
{
    // Check if the audio already exists.
    m_iterator = m_soundSources.find(name);
    if (m_iterator == m_soundSources.end())
    {
        irrklang::ISoundSource* newSoundSrc = m_soundEngine->addSoundSourceFromFile(filePath.c_str());
        m_soundSources.emplace(name, newSoundSrc);
        LOG_INFO("'%s' added to SoundHandler", name.c_str())
    }
    else
    {
        LOG_WARNING("failed to add '%s' to SoundHandler", name.c_str())
    }
}

irrklang::ISound* SoundHandler::Create2DSound(const std::string& name, bool playSound)
{
    irrklang::ISound * sound = nullptr;

    m_iterator = m_soundSources.find(name);
    if (m_iterator != m_soundSources.end())
    {
    	sound = m_soundEngine->play2D(m_soundSources[name], false, true, false, false);
        sound->setIsPaused(!playSound);
        LOG_INFO("created a new instance of '%s'", name.c_str())
    }

    return sound;
}

irrklang::ISound* SoundHandler::CreateUnique2DSound(const std::string& name, bool playSound)
{
    irrklang::ISound* sound = nullptr;

    m_iterator = m_soundSources.find(name);
    if (m_iterator != m_soundSources.end())
    {
        if(!m_soundEngine->isCurrentlyPlaying(m_iterator->second))
        {
	        sound = m_soundEngine->play2D(m_soundSources[name], false, true, false, false);
	        sound->setIsPaused(!playSound);
	        LOG_INFO("created a new instance of '%s'", name.c_str())
        }
    }

    return sound;
}

void SoundHandler::Toggle2DMusic(const std::string& name)
{
    m_iterator = m_soundSources.find(name);
    if (m_iterator != m_soundSources.end())
    {
        if(m_soundEngine->isCurrentlyPlaying(m_iterator->second))
        {
            m_soundEngine->stopAllSoundsOfSoundSource(m_iterator->second);
        }
        else
        {
            m_soundEngine->play2D(m_iterator->second);
        }

        LOG_INFO("toggled '%s'", name.c_str())
    }
}

void SoundHandler::Set2DMusic(const std::string& name, bool playMusic)
{
    m_iterator = m_soundSources.find(name);
    if (m_iterator != m_soundSources.end())
    {
        const bool isPlaying = m_soundEngine->isCurrentlyPlaying(m_iterator->second);

        if (isPlaying && !playMusic)
        {
            m_soundEngine->stopAllSoundsOfSoundSource(m_iterator->second);
        }
        else if (!isPlaying && playMusic)
        {
            m_soundEngine->play2D(m_iterator->second);
        }
    }
}


