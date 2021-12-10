#pragma once
#include <thread>
#include "Renderer.h"

struct render_instructions_t
{
	int start = 0;
	int stop = 0;
};

namespace thread
{
	/*
	The must-meet threshold for divided rendering.
		*Per thread object render*
	
	This means the amount of objects per thread that needs to be met
	for it to be considered benefitable to divide up the rendering
	between threads.
	*/
	const int BASE_THRESHOLD = 1;
	const int SHADOW_THRESHOLD = 1;

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
		void* m_objects;
		void* m_camera;
		void* m_shadows;
		bool m_isActive;

		unsigned int m_amount;
		RenderThreadHandler();
		~RenderThreadHandler();

		/*
			Vector in charge of distributing jobs to threads.
		*/
		std::vector<std::function<void(void*, void*)>> m_jobs;


		/*
			Join all the threads.
		*/
		static void Finish();

		static const int GetStatus(const unsigned int& id);

		static void InsertRenderJob(std::function<void(void*, void*)> job);

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
			1/true - Render on same thread.
			0/false - Rendered on separate threads.
		*/
		static const render_instructions_t LaunchOpaque(const int& amount_of_objects);
		static const render_instructions_t LaunchTransparent(const int& amount_of_objects);

		/*
			
		*/
		static const render_instructions_t DoShadows(const int& amount_of_objects);

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
		static std::function<void(void*, void*)> GetJob();

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

		/*
			Set the reference to the buffer containing all the objects.
		*/
		static void SetObjectsBuffer(void* objects);

		/*
			Get the object buffer.
		*/
		static void* GetObjectsBuffer();

		static void SetCamera(void* camera);
		static void* GetCamera();

		static void SetShadows(void* shadows);
		static void* GetShadows();
	};
}