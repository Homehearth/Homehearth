#include "EnginePCH.h"
#include "RenderThreadHandler.h"

#define INSTANCE thread::RenderThreadHandler::Get()
bool thread::RenderThreadHandler::m_isRunning = false;

CRITICAL_SECTION criticalSection;

void RenderThread(const unsigned int& id);

void RenderJob(const unsigned int start, const unsigned int stop, void* objects);

thread::RenderThreadHandler::RenderThreadHandler()
{
	m_workerThreads = nullptr;
	m_amount = 0;
	m_statuses = nullptr;
}

thread::RenderThreadHandler::~RenderThreadHandler()
{
	INSTANCE.m_isRunning = false;
	if (m_workerThreads)
	{

		delete[] INSTANCE.m_workerThreads;
		delete[] INSTANCE.m_statuses;
	}
}

std::function<void(const unsigned int, const unsigned int, void*)> thread::RenderThreadHandler::GetJob()
{
	return INSTANCE.m_jobs[(int)INSTANCE.m_jobs.size() - 1];
}

void thread::RenderThreadHandler::PopJob()
{
	INSTANCE.m_jobs.pop_back();
}

void thread::RenderThreadHandler::Setup(const int& amount)
{
	INSTANCE.m_isRunning = true;
	INSTANCE.m_amount = amount;
	INSTANCE.m_workerThreads = new std::thread[amount];
	INSTANCE.m_statuses = new unsigned int[amount];

	for (unsigned int i = 0; i < (unsigned int)amount; i++)
	{
		std::thread worker(RenderThread, i);
		INSTANCE.m_workerThreads[i] = std::move(worker);
	}
}

const int thread::RenderThreadHandler::Launch(const int& amount_of_objects, void* objects)
{
	unsigned int objects_per_thread = (amount_of_objects / INSTANCE.m_amount);
	if (objects_per_thread >= thread::threshold)
	{
		// Convert 
		DoubleBuffer<std::vector<comp::Renderable>>* m_objects = (DoubleBuffer<std::vector<comp::Renderable>>*)objects;

		unsigned int iterations = (amount_of_objects / objects_per_thread) - 1;

		for (int i = 0; i < iterations; i++)
		{
			//std::function<void(const unsigned int, const unsigned int, void*)> job = RenderJob(i, i + 1, m_objects);
			//INSTANCE.m_jobs.push_back();
		}

		return 1;
	}

	return 0;
}

const bool thread::RenderThreadHandler::GetHandlerStatus()
{
	return INSTANCE.m_isRunning;
}

void thread::RenderThreadHandler::UpdateStatus(unsigned int& id, unsigned int& status)
{

}

void RenderThread(const unsigned int& id)
{
	while (INSTANCE.GetHandlerStatus())
	{

	}
}

void RenderJob(const unsigned int start,
	const unsigned int stop, void* objects)
{
	
}
