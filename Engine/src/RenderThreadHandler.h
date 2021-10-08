#pragma once
#include <thread>
#include "Renderer.h"

namespace thread
{
	// The must-meet threshold for divided rendering.
	const int threshold = 10;

	class RenderThreadHandler
	{
	private:

		std::thread* m_workerThreads;
		std::vector<ID3D11CommandList*> m_commands;
		Renderer* m_renderer;
		Window* m_window;
		unsigned int* m_statuses;
		bool m_isRunning;
		bool m_isPooled = false;

		unsigned int m_amount;
		RenderThreadHandler();
		~RenderThreadHandler();

		/*
			Vector in charge of distributing jobs to threads.
		*/
		std::vector<std::function<void(void*, void*, void*)>> m_jobs;


		/*
			Join all the threads.
		*/
		static void Finish();

		static const int GetStatus(const unsigned int& id);

		static void InsertRenderJob(std::function<void(void*, void*, void*)> job);

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
			0 - Render on same thread.
			1 - Rendered on separate threads.
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
		static void UpdateStatus(const unsigned int& id, unsigned int status);

		/*
			Get a job from the queue.
		*/
		static std::function<void(void*, void*, void*)> GetJob();

		/*
			Pop the recent job from queue.
		*/
		static void PopJob();

		/*
			Get the amount of jobs from the queue.
		*/
		static const unsigned int GetAmountOfJobs();

		/*
			Set the reference to renderer object.
		*/
		static void SetRenderer(Renderer* rend);
		static Renderer* GetRenderer();

		/*
			Set the reference to window object.
		*/
		static void SetWindow(Window* wind);
		static Window* GetWindow();

		/*
			Insert a command list into the stack.
		*/
		static void InsertCommandList(ID3D11CommandList* list);

		/*
			Execute each command list available on the stack.
		*/
		static void ExecuteCommandLists();
	};
}