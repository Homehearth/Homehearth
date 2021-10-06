#pragma once
#include <thread>

namespace thread
{
	// The must-meet threshold for divided rendering.
	const int threshold = 100;

	class RenderThreadHandler
	{
	private:

		std::thread* m_workerThreads;
		unsigned int* m_statuses;
		static bool m_isRunning;

		unsigned int m_amount;
		RenderThreadHandler();
		~RenderThreadHandler();

		/*
			Vector in charge of distributing jobs to threads.
		*/
		std::vector<std::function<void( 
			const unsigned int, const unsigned int, void*)>> m_jobs;

		/*
			Get a job from the queue.
		*/
		static std::function<void(
			const unsigned int, const unsigned int, void*)> GetJob();

		static void PopJob();

	public:

		static auto& Get()
		{
			static RenderThreadHandler instance;
			return instance;
		}

		static void Setup(const int& amount);

		/*
			Launches threads to take care of divided rendering.
			Retval:
			1 - Render on one thread.
			0 - Rendered on multiple threads.
		*/
		static const int Launch(const int& amount_of_objects, void* objects);

		/*
			Returns a bool that tells if the handler is currently running
			or not.
		*/
		static const bool GetHandlerStatus();

		/*
			Update the status of thread with id.
		*/
		static void UpdateStatus(unsigned int& id, unsigned int& status);
	};
}