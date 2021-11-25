#pragma once


// todo: interface for 3D sounds.


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
	irrklang::ISound* Create2DSound(const std::string& name, bool playSound = true);

	/**
	 * \brief Creates and plays a new unique instance of sound with given sound source name.
	 * This means that sounds of this type will not get overlapped.
	 * \param name The name of the sound source.
	 * \param playSound Dictates if the sound should play upon being created.
	 * \return Returns a pointer to the created and played sound.
	 */
	irrklang::ISound* CreateUnique2DSound(const std::string& name, bool playSound = true);

	/**
	 * \brief Toggles (on/off) the playing state of given music name.
	 * \param name The name of the music source.
	 */
	void Toggle2DMusic(const std::string& name);

	/**
	 * \brief Sets the playing state of given music name.
	 * \param name The name of the music source.
	 * \param playMusic The state to be set.
	 */
	void Set2DMusic(const std::string& name, bool playMusic = true);
};

