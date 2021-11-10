#pragma once
#include <iostream>
#include <ostream>
#include <unordered_map>
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

private:
	Blackboard() = default;
	std::unordered_map<std::string, std::unique_ptr<Base>> storage;
};

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

