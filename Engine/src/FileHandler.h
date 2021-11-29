#pragma once
#include <string>

enum class FileFormat : UINT
{
	// Formatted for options read.
	OPTIONS,

	// Same as Read() without any format.
	DEFAULT
};

class FileHandler
{
private:

	std::fstream m_stream;
	std::string m_filePath;

public:

	FileHandler();
	~FileHandler() = default;

	/*
		Read the entire file linked to filePath and
		get output through parameter vector.
		This read is NOT formatted and you will get entire line unfiltered.
	*/
	bool Read(std::vector<std::string>& input);

	/*
		Read the entire file linked to file path and get output
		through parameter vector.
		This read IS formatted depending on your format choice.
	*/
	bool Read(std::vector<std::string>& input, const FileFormat& format = FileFormat::DEFAULT);

	/*
		Dump/write a vector of string onto a txt file.
	*/
	template<typename T>
	bool Dump(std::vector<T>& output);

	template<typename T>
	bool Dump(std::vector<T>& output, const FileFormat& format);
	
	/*
		Open a file. If a file already is open it will be closed.
	*/
	void Open(const std::string& filePath, const bool& isInput);

	/*
		Close a file.
	*/
	void Close();
};

template<typename T>
inline bool FileHandler::Dump(std::vector<T>& output)
{
	//static_assert(std::is_standard_layout<T>::value, "Data is too complex for output");

	if (m_stream.is_open())
	{
		m_stream.close();
	}

	m_stream.open(m_filePath, std::ios_base::out);
	if (!m_stream.is_open())
	{
		LOG_INFO("Couldn't switch to output on file.");
		return false;
	}

	for (int i = 0; i < output.size(); i++)
	{
		m_stream << output[i];
	}

	return true;
}

template<typename T>
inline bool FileHandler::Dump(std::vector<T>& output, const FileFormat& format)
{
	//static_assert(std::is_standard_layout<T>::value, "Data is too complex for output");

	if (m_stream.is_open())
	{
		m_stream.close();
	}

	m_stream.open(m_filePath, std::ios_base::out);
	if (!m_stream.is_open())
	{
		LOG_WARNING("Couldn't switch to output on file.");
		return false;
	}

	switch (format)
	{
	case FileFormat::DEFAULT:
	{
		for (int i = 0; i < output.size(); i++)
		{
			m_stream << output[i];
		}
		break;
	}
	case FileFormat::OPTIONS:
	{
		for (size_t i = 0; i < output.size(); i+= 2)
		{
			m_stream << output[i] + " " + output[i + 1] + "\n";
		}
		break;
	}
	default:
		LOG_WARNING("File format was ambigous.");
	}

	return true;
}
