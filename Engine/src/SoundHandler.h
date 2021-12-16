#pragma once

/**
 * \brief A custom made wrapper around irrKlang 1.6.0 sound library.
 * In its current stage it holds ISoundSources and 
 */
class SoundHandler
{
private:
	SoundHandler();
	SoundHandler(const SoundHandler& other) = delete;
	SoundHandler& operator=(const SoundHandler& other) = delete;

	irrklang::ISoundEngine* m_soundEngine;
	float m_masterVolume;

	std::unordered_map<std::string, irrklang::ISoundSource*> m_soundSources;
	std::unordered_map<std::string, irrklang::ISoundSource*>::iterator m_iterator;

	irrklang::ISound* m_currentMusic;
	
	void UpdateCurrentMusic(irrklang::ISoundSource* music, bool loopMusic);

	void LoadAllSounds();

public:
	virtual ~SoundHandler();

	/**
	 * \brief Creates and/or returns an instance of the SoundHandler class.
	 * \return Returns an instance of the SoundHandler class.
	 */
	static SoundHandler& Get();

	void Setup();

	void Update();

	void AddSoundSource(const std::string& filePath);

	void AddSoundSource(const std::string& filePath, const std::string& name);

	void SetCurrentMusic(const std::string& name, bool loopMusic = true);

	irrklang::ISound* GetCurrentMusic();

	void SetListenerPosition(const sm::Vector3& pos, const sm::Vector3& lookDir);

	irrklang::ISound* PlaySound(const std::string &name, const audio_t &data);

	void SetMasterVolume(float value);
	const float& GetMasterVolume() const;
	float& AdjustMasterVolume();
};
