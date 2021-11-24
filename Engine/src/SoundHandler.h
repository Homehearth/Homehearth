#pragma once

/**
 * \brief A custom made wrapper around irrKlang 1.6.0 sound library.
 */
class SoundHandler
{
private:
	SoundHandler();

	irrklang::ISoundEngine* m_engine;

	std::unordered_map<std::string, irrklang::ISoundSource*> m_sources;

	// Testing.
	irrklang::ISoundSource* m_ss;
	irrklang::ISoundSource* m_ss2;
public:
	virtual ~SoundHandler();

	/**
	 * \brief Creates and/or returns an instance of the SoundHandler class.
	 * \return Returns an instance of the SoundHandler class.
	 */
	static SoundHandler& Get();


	/**
	 * \brief Load a sound from a file and adds it to the SoundHandler.
	 * \param filePath The folder location of the file, including file format.
	 */
	void AddSound(const std::string& filePath);

	/**
	 * \brief Various functions to test irrKlang and different file formats.
	 */
	void PlayOnClick();
	void PlayMainMenuTheme();
};
