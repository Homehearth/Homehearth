#pragma once

class BackBuffer
{
private:

	static BackBuffer* instance;
	DoubleBuffer<std::unordered_map<entt::entity, comp::Renderable>> m_buffers;

public:

	static void Initialize();
	static void Destroy();

	/*
		Returns a pointer to the back buffers
	*/
	static DoubleBuffer<std::unordered_map<entt::entity, comp::Renderable>>* GetBuffers();
};