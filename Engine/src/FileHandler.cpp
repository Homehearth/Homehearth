#include "EnginePCH.h"
#include "FileHandler.h"
#include <sstream>

FileHandler::FileHandler()
{
}

bool FileHandler::Read(std::vector<std::string>& input)
{
	if (!m_stream.is_open())
	{
		LOG_WARNING("File handler has no file open!");
		return false;
	}

	// Read line by line and push into vector.
	std::string line;
	while (std::getline(m_stream, line))
	{
		input.push_back(line);
	}

	return true;
}

bool FileHandler::Read(std::vector<std::string>& input, const FileFormat& format)
{
	if (!m_stream.is_open())
	{
		LOG_WARNING("File handler has no file open!");
		return false;
	}

	switch (format)
	{
	case FileFormat::OPTIONS:
	{
		// Read line by line and push into vector formatted for options.
		std::string line;
		while (std::getline(m_stream, line))
		{
			std::stringstream sStream(line);
			std::string type;
			sStream >> type;
			std::string value;
			sStream >> value;
			input.push_back(type);
			input.push_back(value);
		}
		break;
	}
	case FileFormat::DEFAULT:
	{
		// Read line by line and push into vector.
		std::string line;
		while (std::getline(m_stream, line))
		{
			input.push_back(line);
		}
		break;
	}
	}
	return false;
}

void FileHandler::Open(const std::string& filePath, const bool& isInput)
{
	// Safety check.
	if (m_stream.is_open())
	{
		m_stream.close();
	}

	switch (isInput)
	{
	case true:
	{
		m_stream.open(filePath, std::fstream::in);
		break;
	}
	case false:
	{
		m_stream.open(filePath, std::fstream::out);
		break;
	}
	}

	if (!m_stream.is_open())
	{
		//LOG_WARNING("Couldn't open file %s", filePath.c_str());

		// Create file if not existing.
		m_stream.open(filePath, std::fstream::app);
		if (m_stream.is_open())
			m_stream.close();
		m_filePath = filePath;
	}
	else
		m_filePath = filePath;
}

void FileHandler::Close()
{
	m_stream.close();
}
