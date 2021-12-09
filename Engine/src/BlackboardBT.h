#pragma once
#include <iostream>
#include <ostream>
#include <unordered_map>
#include "PathFinderManager.h"

struct Base
{
	virtual ~Base() = default;
	virtual void virtualFunction() = 0;
};

template <typename T>
struct Derived final : Base
{
	void virtualFunction() override {};
	T value;
};

//Structs of different blackboard data.
struct PlayersPosition_t
{
	std::vector<Entity> players{};
};

struct Houses_t
{
	std::unordered_map<Entity, Entity> houses{};
};



class Blackboard final
{
public:
	Blackboard() = default;
	virtual ~Blackboard() = default;

	template<typename T>
	T* GetValue(const std::string& key);

	template<typename T>
	bool AddValue(const std::string& key, T value);
	PathFinderManager* GetPathFindManager();

	template <typename T>
	void ClearValue(const std::string& key);
private:
	std::unordered_map<std::string, std::unique_ptr<Base>> storage{};
	PathFinderManager pathFindManager;
};

inline PathFinderManager* Blackboard::GetPathFindManager()
{
	return &this->pathFindManager;
}

template <typename T>
void Blackboard::ClearValue(const std::string& key)
{
	const auto iterator = storage.find(key);

	if (iterator != storage.end())
	{
		storage.erase(iterator);
	}
}

template <typename T>
T* Blackboard::GetValue(const std::string& key)
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
bool Blackboard::AddValue(const std::string& key, T value)
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

