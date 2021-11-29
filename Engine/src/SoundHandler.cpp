#include "EnginePCH.h"
#include "SoundHandler.h"

SoundHandler::SoundHandler()
	: m_currentMusic(nullptr)
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
}

void SoundHandler::UpdateCurrentMusic(irrklang::ISoundSource* musicSrc, bool loopMusic)
{
    if(m_currentMusic != nullptr)
    {
        m_currentMusic->stop();
        m_currentMusic->drop();
    }

    m_currentMusic = m_soundEngine->play2D(musicSrc, loopMusic, true, false, false);
    m_currentMusic->setIsPaused(false);
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
        if(newSoundSrc != nullptr)
        {
			m_soundSources.emplace(name, newSoundSrc);
			LOG_INFO("'%s' added to SoundHandler", name.c_str())
        }
        else
        {
            LOG_WARNING("failed to add '%s' to SoundHandler", name.c_str())
        }
    }
}

irrklang::ISound* SoundHandler::Play2DSound(const std::string& name, bool playSound)
{
    irrklang::ISound * sound = nullptr;

    m_iterator = m_soundSources.find(name);
    if (m_iterator != m_soundSources.end())
    {
    	sound = m_soundEngine->play2D(m_soundSources[name], false, true, false, false);
        sound->setIsPaused(!playSound);
    }

    return sound;
}

irrklang::ISound* SoundHandler::PlayUnique2DSound(const std::string& name, bool playSound)
{
    irrklang::ISound* sound = nullptr;

    m_iterator = m_soundSources.find(name);
    if (m_iterator != m_soundSources.end())
    {
        if(!m_soundEngine->isCurrentlyPlaying(m_iterator->second))
        {
	        sound = m_soundEngine->play2D(m_soundSources[name], false, true, false, false);
	        sound->setIsPaused(!playSound);
        }
    }

    return sound;
}

void SoundHandler::SetCurrentMusic(const std::string& name, bool loopMusic)
{
    m_iterator = m_soundSources.find(name);
    if (m_iterator != m_soundSources.end())
    {
        const bool isPlaying = m_soundEngine->isCurrentlyPlaying(m_iterator->second);
        if (isPlaying)
        {
            m_soundEngine->stopAllSoundsOfSoundSource(m_iterator->second);
        }
        
        UpdateCurrentMusic(m_iterator->second, loopMusic);
    }
}

irrklang::ISound* SoundHandler::GetCurrentMusic()
{
    return m_currentMusic;
}

irrklang::ISound* SoundHandler::Play3DSound(const std::string& name, const sm::Vector3 &pos, bool playSound)
{
    irrklang::ISound* sound = nullptr;

    m_iterator = m_soundSources.find(name);
    if (m_iterator != m_soundSources.end())
    {
        sound = m_soundEngine->play3D(m_soundSources[name],{ pos.x, pos.y, pos.z },
            false, true, false, false);
        sound->setMinDistance(500.f);
        sound->setIsPaused(!playSound);
    }

    return sound;
}

irrklang::ISound* SoundHandler::PlayUnique3DSound(const std::string& name, const sm::Vector3& pos, bool playSound)
{
    irrklang::ISound* sound = nullptr;

    m_iterator = m_soundSources.find(name);
    if (m_iterator != m_soundSources.end())
    {
        if (!m_soundEngine->isCurrentlyPlaying(m_iterator->second))
        {
            sound = m_soundEngine->play3D(m_soundSources[name], { pos.x, pos.y, pos.z },
                false, true, false, false);
            sound->setIsPaused(!playSound);
        }
    }

    return sound;
}

void SoundHandler::SetListenerPosition(const sm::Vector3& pos, const sm::Vector3& lookDir)
{
    m_soundEngine->setListenerPosition({ pos.x, pos.y, pos.z },
        { lookDir.x, lookDir.y, lookDir.z });
}

void SoundHandler::AddToQueue(const audio_t& audio)
{
    m_audioQueue.emplace(audio);
}

std::queue<audio_t> SoundHandler::GetQueue()
{
    return m_audioQueue;
}


