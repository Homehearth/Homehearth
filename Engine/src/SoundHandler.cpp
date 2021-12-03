#include "EnginePCH.h"
#include "SoundHandler.h"

SoundHandler::SoundHandler()
	: m_currentMusic(nullptr)
{
    m_soundEngine = nullptr;
    m_masterVolume = 0.3f;
}

void SoundHandler::UpdateCurrentMusic(irrklang::ISoundSource* musicSrc, bool loopMusic)
{
    if(m_currentMusic != nullptr)
    {
        m_currentMusic->stop();
        m_currentMusic->drop();
        m_currentMusic = nullptr;
    }

    m_currentMusic = m_soundEngine->play2D(musicSrc, loopMusic, true, false, false);
    m_currentMusic->setVolume(m_masterVolume);
    m_currentMusic->setIsPaused(false);
}

void SoundHandler::LoadAllSounds()
{
    AddSoundSource("../Assets/Sounds/DayTheme.wav");
    AddSoundSource("../Assets/Sounds/NightTheme.wav");
    AddSoundSource("../Assets/Sounds/MenuTheme.wav");
    AddSoundSource("../Assets/Sounds/BossTheme.wav");
    AddSoundSource("../Assets/Sounds/ButtonClick.wav"); // is placed in Button.cpp.
    AddSoundSource("../Assets/Sounds/OnGameOver.wav");

    AddSoundSource("../Assets/Sounds/Game_OnDefencePlaced.mp3");
    AddSoundSource("../Assets/Sounds/Game_OnDefenceDestroyed.wav");
    AddSoundSource("../Assets/Sounds/Game_OnHouseDestroyed.wav");
    AddSoundSource("../Assets/Sounds/Game_OnBossSpawn.wav");
    AddSoundSource("../Assets/Sounds/Game_OnPurchase.mp3");

    AddSoundSource("../Assets/Sounds/Player_OnMovement.wav");
    AddSoundSource("../Assets/Sounds/Player_OnMeleeAttack.wav");
    AddSoundSource("../Assets/Sounds/Player_OnMeleeAttackHit1.wav");
    AddSoundSource("../Assets/Sounds/Player_OnMeleeAttackHit2.wav");
    AddSoundSource("../Assets/Sounds/Player_OnMeleeAttackHit3.wav");
    AddSoundSource("../Assets/Sounds/Player_OnRangeAttack.wav");
    AddSoundSource("../Assets/Sounds/Player_OnRangeAttackHit.wav");
    AddSoundSource("../Assets/Sounds/Player_OnDmgDealt.wav");
    AddSoundSource("../Assets/Sounds/Player_OnDmgRecieved.wav");
    AddSoundSource("../Assets/Sounds/Player_OnHealing.wav");
    AddSoundSource("../Assets/Sounds/Player_OnCastDash.wav");
    AddSoundSource("../Assets/Sounds/Player_OnCastDash1.wav");
    AddSoundSource("../Assets/Sounds/Player_OnCastBlink.wav");
    AddSoundSource("../Assets/Sounds/Player_OnDeath.wav");
    AddSoundSource("../Assets/Sounds/Player_OnRespawn.wav");    // do not exist

    AddSoundSource("../Assets/Sounds/Enemy_OnMeleeAttack1.mp3");
    AddSoundSource("../Assets/Sounds/Enemy_OnMeleeAttack2.mp3");
    AddSoundSource("../Assets/Sounds/Enemy_OnMeleeAttack3.mp3");
    AddSoundSource("../Assets/Sounds/Enemy_OnMeleeAttack4.mp3");
    AddSoundSource("../Assets/Sounds/Enemy_OnMeleeAttack5.mp3");
    AddSoundSource("../Assets/Sounds/Enemy_OnMeleeAttack6.mp3");
    AddSoundSource("../Assets/Sounds/Enemy_OnMeleeAttack7.wav");
    AddSoundSource("../Assets/Sounds/Enemy_OnDmgRecieved.ogg");
    AddSoundSource("../Assets/Sounds/Enemy_OnDeath.wav");   
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

void SoundHandler::Setup()
{
    // Start the sound engine with default parameters.
    m_soundEngine = irrklang::createIrrKlangDevice();

    if (m_soundEngine == nullptr)
    {
        LOG_ERROR("failed starting up the irrklang sound engine.")
    }
    else
    {
        m_soundEngine->setDefault3DSoundMinDistance(100.0f);
        m_soundEngine->setRolloffFactor(70.0f);
        m_soundEngine->setSoundVolume(m_masterVolume);

        LOG_INFO("Irrklang sound engine successfully loaded.")
    }

    LoadAllSounds();
}

void SoundHandler::Update()
{
    m_soundEngine->setSoundVolume(m_masterVolume);
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

void SoundHandler::SetCurrentMusic(const std::string& name, bool loopMusic)
{
    m_iterator = m_soundSources.find(name);
    if (m_iterator != m_soundSources.end())
    {
        if (!m_soundEngine->isCurrentlyPlaying(m_iterator->second))
        {
            LOG_INFO("'%s' is set as current music", name.c_str())
        	UpdateCurrentMusic(m_iterator->second, loopMusic);
        }
    }
}

irrklang::ISound* SoundHandler::GetCurrentMusic()
{
    return m_currentMusic;
}

void SoundHandler::SetListenerPosition(const sm::Vector3& pos, const sm::Vector3& lookDir)
{
    m_soundEngine->setListenerPosition({ pos.x, pos.y, pos.z },
        { lookDir.x, lookDir.y, lookDir.z });
}

irrklang::ISound* SoundHandler::PlaySound(const std::string& name, const audio_t& data)
{
    irrklang::ISound* sound = nullptr;

    // Check if Sound Src exists.
    m_iterator = m_soundSources.find(name);
    if (m_iterator != m_soundSources.end())
    {
        // isUnique condition check.
        if (m_soundEngine->isCurrentlyPlaying(m_iterator->second) && data.isUnique)
            return sound;

        if(data.is3D)
        {
            sound = m_soundEngine->play3D(m_soundSources[name], { data.position.x, data.position.y, data.position.z },
                data.playLooped, true, false, false);

            const float minDist = (data.minDistance <= 0) ? m_soundEngine->getDefault3DSoundMinDistance() : data.minDistance;
            float volume = (data.volume <= 0.1f) ? 1.0f : data.volume;
            volume *= m_masterVolume;

            sound->setMinDistance(minDist);
            sound->setVolume(data.volume);
            sound->setIsPaused(false);
        }
        else
        {
            sound = m_soundEngine->play2D(m_soundSources[name], data.playLooped, true, false, false);
            float volume = (data.volume <= 0.1f) ? 1.0f : data.volume;
            volume *= m_masterVolume;
        	sound->setVolume(volume);
            sound->setIsPaused(false);
        }
    }

    return sound;
}

void SoundHandler::SetMasterVolume(float value)
{
    m_masterVolume = value;
}

const float& SoundHandler::GetMasterVolume() const
{
    return m_masterVolume;
}

float& SoundHandler::AdjustMasterVolume()
{
    return m_masterVolume;
}


