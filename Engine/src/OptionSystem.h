#pragma once
#include <string>
#include "FileHandler.h"

class OptionSystem
{
private:

	FileHandler m_fileHandler;
	std::vector<std::string> m_options;

public:

	static void OnStartUp();
	static void OnShutdown();

	static auto& Get()
	{
		static OptionSystem instance;
		return instance;
	};

	/*
		Return the value for specified option name.
		"0" is returned upon non existing optionName.
	*/
	static std::string GetOption(const std::string& optionName);

	/*
		Edit the value of an option parameter.
	*/
	static bool SetOption(const std::string& optionName, std::string& value);
};