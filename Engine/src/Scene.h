#pragma once
#include "EventTypes.h"

#include "Components.h"

#include <mutex>

template<typename T>
class Double {
private:
	T m_data[2];
	std::atomic<bool> m_isSwapped;
	std::mutex m_mutex;
public:
	Double()
		: m_isSwapped(false)
	{
	}

	T& operator[](short i)
	{
		return m_data[i];
	}

	void Swap()
	{
		m_mutex.lock();
		std::swap(m_data[0], m_data[1]);
		m_mutex.unlock();
		m_isSwapped = true;
	}
	
	void ReadyForSwap() 
	{
		m_isSwapped = false;
	}

	bool IsSwapped() const
	{
		return m_isSwapped;
	}
};

class Scene : public entt::emitter<Scene>
{
private:
	// Registry handles all ecs data
	entt::registry m_registry;
	
	Double<std::unordered_map<entt::entity, comp::Transform>> m_transformCopies;
	

public:

	Scene();
	virtual ~Scene() = default;
	Scene(const Scene&) = delete;
	void operator=(const Scene&) = delete;

	entt::registry& GetRegistry();

	// Emit update event and update constant buffers
	void Update(float dt);

	// Emit render event and render Renderable components
	void Render();

	bool IsRenderReady() const;

};
