#include <NetServerPCH.h>
#include "multi_thread_manager.h"
#include <iostream>

using namespace thread;
#define POOL(x) MultiThreader::Pool(x)
#define MULTITHREADER MultiThreader::instance

MultiThreader* MultiThreader::instance = nullptr;


// Shared mutex
std::mutex mutx;

#ifdef _DEBUG
// Shared mutex for io print.
std::mutex io_mtx;
#endif

/*
	Main function for Pooled threads.
*/
void PooledThread(unsigned int id)
{
	MultiThreader::SetStatus(thread_running, id);


//#ifdef _DEBUG
//	io_mtx.lock();
//	std::cout << "Thread started with ID: " << id << "\n";
//	io_mtx.unlock();
//#endif

	// Main active Loop
	bool active = true;
	while (active)
	{
		// Mutex protects Job list, Only one job per thread.
		mutx.lock();

		// Critical Zone.
		std::function<void()> currentJob = MultiThreader::CheckJob();
		if (currentJob != nullptr)
		{
			MultiThreader::SetStatus(thread_working, id);
#ifdef _DEBUG
			
			//io_mtx.lock();
			std::cout << "Found a Job!\n";
			//io_mtx.unlock();
#endif

			MultiThreader::PopJob();
			mutx.unlock();

			// Runs the currentJob.
			currentJob();

			MultiThreader::SetStatus(thread_running, id);
		}
		else
		{

			mutx.unlock();

		}

		// Update end of life.
		active = MultiThreader::IsActive();
	}

	MultiThreader::SetStatus(thread_done, id);
}

MultiThreader::MultiThreader()
{
	this->activeThreads = 0;
	this->statuses = nullptr;
	this->pooledThreads = nullptr;
	this->pooled = 0;
	this->isActive = false;
	this->type = ThreadType::DEFAULT;
}

MultiThreader::~MultiThreader()
{
	this->isActive = false;
	// Empty.
}

void MultiThreader::Init(unsigned int amount, ThreadType type)
{
	if (MULTITHREADER == nullptr)
	{
		if (amount <= 0)
			return;

		MULTITHREADER = new MultiThreader;
		MULTITHREADER->activeThreads = 0;

		MULTITHREADER->statuses = new unsigned int[availableCores];
		for (int i = 0; i < availableCores; i++)
		{
			MULTITHREADER->statuses[i] = thread_free;
		}

		// Create the pool.
		MULTITHREADER->type = type;
		// LIFO job assignment.
		if (MULTITHREADER->type == ThreadType::POOL_LIFO || MULTITHREADER->type == ThreadType::POOL_FIFO)
		{
			POOL(amount);
		}
	}
}

void MultiThreader::Destroy()
{
	if (!MULTITHREADER)
		return;
	// Set activity to false to signal all threads to shutdown.
	MULTITHREADER->isActive = false;
	for (int i = 0; i < (int)MULTITHREADER->threads.size(); i++)
	{
		// Join the threads or simply detach it and get rid of it.
		if (MULTITHREADER->threads[i].joinable())
			MULTITHREADER->threads[i].join();
		else
			MULTITHREADER->threads[i].detach();
	}
	for (int i = 0; i < (int)MULTITHREADER->pooled; i++)
	{
		if (MULTITHREADER->pooledThreads[i].joinable())
			MULTITHREADER->pooledThreads[i].join();
		else
			MULTITHREADER->pooledThreads[i].detach();
	}
	delete[] MULTITHREADER->pooledThreads;
	delete[] MULTITHREADER->statuses;
	delete MULTITHREADER;
}

void MultiThreader::Pool(unsigned int amount)
{
	if (amount <= (unsigned int)availableCores && amount > 0)
	{
		MULTITHREADER->isActive = true;
		MULTITHREADER->pooled = amount;
		MULTITHREADER->pooledThreads = new std::thread[amount];
		for (unsigned int i = 0; i < amount; i++)
		{
			std::thread newThread(PooledThread, i);
			MULTITHREADER->pooledThreads[i] = std::move(newThread);
			MULTITHREADER->activeThreads++;
		}
	}
	else
	{
		MULTITHREADER->isActive = true;
		MULTITHREADER->pooled = availableCores;
		MULTITHREADER->pooledThreads = new std::thread[availableCores];
		for (unsigned int i = 0; i < (unsigned int)availableCores; i++)
		{
			std::thread newThread(PooledThread, i);
			MULTITHREADER->pooledThreads[i] = std::move(newThread);
			MULTITHREADER->activeThreads++;
		}
	}
}

int MultiThreader::Start(void(*function)(int))
{
	if (MULTITHREADER->activeThreads < availableCores)
	{
		// Find a free thread within the pool.
		unsigned int freeThread = -1;
		for (int i = 0; i < availableCores; i++)
		{
			if (MULTITHREADER->statuses[i] == thread_free)
			{
				freeThread = i;
				break;
			}
		}

		// If a free thread is found it is then activated with the function.
		if (freeThread != -1)
		{
			std::thread newThread(function, freeThread);

			// A thread is an R-value therefore needs to use move() to actually move it to a vector.
			MULTITHREADER->threads.push_back(std::move(newThread));
			MULTITHREADER->activeThreads++;

			return freeThread;
		}
		else
			return -1;
	}
	else
		return -1;
}

void MultiThreader::Update()
{
	for (int i = 0; i < MULTITHREADER->activeThreads; i++)
	{
		if (MULTITHREADER->statuses[i] == thread_done)
		{
			if (MULTITHREADER->threads[i].joinable())
			{
				MULTITHREADER->threads[i].join();
				MULTITHREADER->activeThreads--;

				// Reset the thread to free status
				MULTITHREADER->statuses[i] = thread_free;
			}
		}
	}

	// Clean up.
	if (MULTITHREADER->activeThreads <= 0 && (int)MULTITHREADER->threads.size() > 0)
	{
		while ((int)MULTITHREADER->threads.size() > 0)
			MULTITHREADER->threads.pop_back();
		MULTITHREADER->activeThreads = 0;

		for (int i = 0; i < availableCores; i++)
			MULTITHREADER->statuses[i] = thread_free;
	}
}

const bool MultiThreader::HasActiveThread()
{
	for (int i = 0; i < availableCores; i++)
	{
		const int status = MULTITHREADER->GetStatus(i);
		if (status == thread_working)
			return true;
	}

	return false;
}

const int MultiThreader::GetStatus(int index)
{
	if (index < availableCores && index >= 0) { return MULTITHREADER->statuses[index]; }
	else return -1;
}

void MultiThreader::SetStatus(unsigned int status, int index)
{
	if (index < availableCores && index >= 0)
	{
		MULTITHREADER->statuses[index] = status;
	}
#ifdef _DEBUG
	else
	{
		std::cout << "You went outside the range of indices, Range is " << 0 << " to " << availableCores << ". Your index was: " << index << "\n";
	}

#endif
}

std::function<void()> MultiThreader::CheckJob()
{
	switch (MULTITHREADER->type)
	{
	case ThreadType::POOL_LIFO:
		if ((int)MULTITHREADER->jobs.size() > 0)
		{
			return MULTITHREADER->jobs[(int)MULTITHREADER->jobs.size() - 1];
		}
		else
			return nullptr;
	break;
	case ThreadType::POOL_FIFO:
		if ((int)MULTITHREADER->jobs_queue.size() > 0)
		{
			return MULTITHREADER->jobs_queue.front();
		}
		else
			return nullptr;
		break;
	default:
		return nullptr;
	break;
	}

	// Nothing found or no ThreadType was specified.
	return nullptr;
}

void MultiThreader::PopJob()
{
	switch (MULTITHREADER->type)
	{
	case ThreadType::POOL_LIFO:
		MULTITHREADER->jobs.pop_back();
		break;
	case ThreadType::POOL_FIFO:
		MULTITHREADER->jobs_queue.pop();
		break;
	default:
		break;
	}
}

void MultiThreader::InsertJob(std::function<void()> job)
{
	switch (MULTITHREADER->type)
	{
	case ThreadType::POOL_LIFO:
		MULTITHREADER->jobs.push_back(job);
		break;
	case ThreadType::POOL_FIFO:
		MULTITHREADER->jobs_queue.push(job);
		break;
	default:
		break;
	}

}

const bool MultiThreader::IsActive()
{
	return MULTITHREADER->isActive;
}

const int MultiThreader::GetAmountOfJobs()
{
	// Used for #define.
	if (!MULTITHREADER->IsActive())
		return 1;

	switch (MULTITHREADER->type)
	{
	case ThreadType::POOL_LIFO:
		return (int)MULTITHREADER->jobs.size();
		break;
	case ThreadType::POOL_FIFO:
		return (int)MULTITHREADER->jobs_queue.size();
		break;
	default:
		return 0;
		break;
	}

	return 0;
}

const bool thread::IsThreadActive()
{
	if (MultiThreader::instance)
		return true;
	else
		return false;
}
