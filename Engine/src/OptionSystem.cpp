#include "EnginePCH.h"
#include "OptionSystem.h"

#define INSTANCE OptionSystem::Get()

void OptionSystem::OnStartUp()
{
	INSTANCE.m_fileHandler.Open(OPTIONPATH, true);
	INSTANCE.m_fileHandler.Read(INSTANCE.m_options, FileFormat::OPTIONS);
}

void OptionSystem::OnShutdown()
{
	INSTANCE.m_fileHandler.Dump(INSTANCE.m_options, FileFormat::OPTIONS);
	INSTANCE.m_fileHandler.Close();
}

std::string OptionSystem::GetOption(const std::string& optionName)
{
	for (size_t i = 0; i < INSTANCE.m_options.size(); i++)
	{
		if (INSTANCE.m_options[i] == optionName)
		{
			if (i + 1 < INSTANCE.m_options.size())
				return INSTANCE.m_options[i + 1];
		}
	}

	return std::string("0");
}

bool OptionSystem::SetOption(const std::string& optionName, std::string& value)
{
	for (size_t i = 0; i < INSTANCE.m_options.size(); i++)
	{
		if (INSTANCE.m_options[i] == optionName)
		{
			if (i + 1 < INSTANCE.m_options.size())
			{
				INSTANCE.m_options[i + 1] = value;
				return true;
			}
		}
	}

	INSTANCE.m_options.push_back(optionName);
	INSTANCE.m_options.push_back(value);

	return false;
}
