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

	std::unordered_map<std::string, irrklang::ISoundSource*> m_soundSources;
	std::unordered_map<std::string, irrklang::ISoundSource*>::iterator m_iterator;

	irrklang::ISound* m_currentMusic;

	std::queue<audio_t> m_audioQueue;
	std::vector<irrklang::ISound*> m_activeSounds;

	void UpdateCurrentMusic(irrklang::ISoundSource* music, bool loopMusic);
public:
	virtual ~SoundHandler();

	/**
	 * \brief Creates and/or returns an instance of the SoundHandler class.
	 * \return Returns an instance of the SoundHandler class.
	 */
	static SoundHandler& Get();

	/**
	 * \brief Updates the audio engine.
	 */
	void Update();

	/**
	 * \brief Load a sound from a file and adds it to the SoundHandler.
	 * \param filePath The folder location of the file, including file format.
	 */
	void AddSoundSource(const std::string& filePath);

	/**
	 * \brief Load a sound from a file and adds it to the SoundHandler.
	 * \param filePath The folder location of the file, including file format.
	 * \param name The name that may be used to fetch the audio.
	 */
	void AddSoundSource(const std::string& filePath, const std::string& name);

	/**
	 * \brief Creates and plays a new instance of sound with given sound source name.
	 * \param name The name of the sound source.
	 * \param playSound Dictates if the sound should play upon being created.
	 * \return Returns a pointer to the created and played sound.
	 */
	irrklang::ISound* Play2DSound(const std::string& name, bool playSound = true);

	/**
	 * \brief Creates and plays a new unique instance of sound with given sound source name.
	 * This means that sounds of this type will not get overlapped.
	 * \param name The name of the sound source.
	 * \param playSound Dictates if the sound should play upon being created.
	 * \return Returns a pointer to the created and played sound.
	 */
	irrklang::ISound* PlayUnique2DSound(const std::string& name, bool playSound = true);

	/**
	 * \brief Set current music. Setting another music will stop current from playing.
	 * \param name The name of the music source.
	 * \param loopMusic Should the music be played looped?.
	 */
	void SetCurrentMusic(const std::string& name, bool loopMusic = true);

	/**
	 * \brief Get the current set music. May be used to set some properties.
	 * OBS: Do not use drop() on the returned ISound.
	 * \return Returns the current set music.
	 */
	irrklang::ISound* GetCurrentMusic();

	/**
	* \brief Creates and plays a new instance of sound with given sound source name.
	* \param name The name of the sound source.
	* \param playSound Dictates if the sound should play upon being created.
	* \return Returns a pointer to the created and played sound.
	*/
	irrklang::ISound* Play3DSound(const std::string& name, const sm::Vector3& pos, bool playSound = true);

	/**
	 * \brief Creates and plays a new unique instance of sound with given sound source name.
	 * This means that sounds of this type will not get overlapped.
	 * \param name The name of the sound source.
	 * \param playSound Dictates if the sound should play upon being created.
	 * \return Returns a pointer to the created and played sound.
	 */
	irrklang::ISound* PlayUnique3DSound(const std::string& name, const sm::Vector3& pos, bool playSound = true);

	/**
	 * \brief Set the position of the Listener, which is required for 3D sounds to work properly.
	 * \param pos The position.
	 * \param lookDir The look direction.
	 */
	void SetListenerPosition(const sm::Vector3& pos, const sm::Vector3& lookDir);

	void AddToQueue(const audio_t &audio);
	std::queue<audio_t> GetQueue();
};
