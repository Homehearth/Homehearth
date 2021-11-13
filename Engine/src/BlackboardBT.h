#pragma once
#include <iostream>
#include <ostream>
#include <unordered_map>
#include "AIHandler.h"

struct Base
{
	virtual void virtualFunction() = 0;
};

template <typename T>
struct Derived : Base
{
	void virtualFunction() override {};
	T value;
};

//Structs of different blackboard data.
struct PlayersPosition_t
{
	std::vector<sm::Vector3> positions;
};


class Blackboard
{
public:
	virtual ~Blackboard() = default;
	static auto& Get()
	{
		static Blackboard s_instance;
		return s_instance;
	}

	template<typename T>
	T* GetValue(std::string key);

	template<typename T>
	bool AddValue(std::string key, T value);

	AIHandler* GetAIHandler();
private:
	Blackboard() = default;
	std::unordered_map<std::string, std::unique_ptr<Base>> storage;
	AIHandler aiHandler;
};

inline AIHandler* Blackboard::GetAIHandler()
{
	return &this->aiHandler;
}

template <typename T>
T* Blackboard::GetValue(std::string key)
{
	const auto iterator = storage.find(key);

	if(iterator != storage.end())
	{
		Derived<T>* derived = dynamic_cast<Derived<T>*>(iterator->second.get());

		if(derived != nullptr)
		{
			return &derived->value;
		}
		else
		{
			std::cout << "Failed to downcast the type..." << std::endl;
			return nullptr;
		}

	}

	return nullptr;
}

template <typename T>
bool Blackboard::AddValue(std::string key, T value)
{
	const auto& iterator = storage.find(key);
	if(iterator == storage.end())
	{
		storage[key] = std::make_unique<Derived<T>>();

		Derived<T>* ptr = dynamic_cast<Derived<T>*>(storage[key].get());

		ptr->value = value;

		return true;
	}
	else
	{
		Derived<T>* ptr = dynamic_cast<Derived<T>*>(iterator->second.get());

		if(ptr != nullptr)
		{
			ptr->value = value;
			return true;
		}
		else
		{
			std::cout << "Wrong value cast..." << std::endl;
		}

		
	}

	return false;
}

