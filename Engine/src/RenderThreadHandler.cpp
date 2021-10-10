#include "EnginePCH.h"
#include "RenderThreadHandler.h"
#include "multi_thread_manager.h"
#include <tgmath.h>


/*
	Crash info:
	Lambda funktionen n�r den vill ska tas ut returnerar empty ibland.
	Anledningen �r funktionen RenderDeferred p� model.
	Varf�r vet jag inte.
*/

#define INSTANCE thread::RenderThreadHandler::Get()
#define CONTEXT D3D11Core::Get().DeviceContext()

CRITICAL_SECTION criticalSection;
std::condition_variable cv;
std::mutex render_mutex;
std::mutex thread_mutex;
std::mutex job_mutex;
bool shouldRender = true;

bool ShouldContinue();
void RenderMain(const unsigned int& id);
void RenderJob(const unsigned int start, unsigned int stop, void* buffer, void* context, void* pipe);

thread::RenderThreadHandler::RenderThreadHandler()
{
	m_workerThreads = nullptr;
	m_amount = 0;
	m_renderer = nullptr;
	m_window = nullptr;
	m_statuses = nullptr;
	m_objects = nullptr;
	m_isRunning = false;
	InitializeCriticalSection(&criticalSection);
}

thread::RenderThreadHandler::~RenderThreadHandler()
{
	INSTANCE.m_isRunning = false;
	cv.notify_all();
	if (m_workerThreads)
	{
		for (int i = 0; i < (int)INSTANCE.m_amount; i++)
		{
			if (INSTANCE.m_workerThreads[i].joinable())
				INSTANCE.m_workerThreads[i].join();
			else
				INSTANCE.m_workerThreads[i].detach();
		}
		delete[] INSTANCE.m_workerThreads;
		delete[] INSTANCE.m_statuses;
	}

	for (int i = 0; i < (int)INSTANCE.m_commands.size(); i++)
	{
		INSTANCE.m_commands[i]->Release();
	}
	INSTANCE.m_jobs.clear();
}

void thread::RenderThreadHandler::Finish()
{
	if (!INSTANCE.m_isPooled)
	{
		for (int i = 0; i < (int)INSTANCE.m_amount; i++)
		{
			INSTANCE.m_workerThreads[i].join();
		}
	}
	else
	{
		// Block until all threads have gotten their jobs.
		while (INSTANCE.m_jobs.size() > 0) { 
			//std::cout << "Size: " << INSTANCE.m_jobs.size() << "\n";
		};

		// Block until all threads have stopped working.
		for (int i = 0; i < (int)INSTANCE.m_amount; i++)
		{
			if (INSTANCE.m_statuses[i] != thread::thread_running)
			{
				i = 0;
			}
		}
	}
}

const int thread::RenderThreadHandler::GetStatus(const unsigned int& id)
{
	return INSTANCE.m_statuses[id];
}

void thread::RenderThreadHandler::InsertRenderJob(std::function<void(void*, void*, void*)> job)
{
	INSTANCE.Get().m_jobs.push_back(job);
}

std::function<void(void*, void*, void*)> thread::RenderThreadHandler::GetJob()
{
	if ((int)INSTANCE.m_jobs.size() <= 0)
	{
		return nullptr;
	}
	const int size = (int)INSTANCE.m_jobs.size() - 1;

	return INSTANCE.m_jobs[size];
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
		std::thread worker(RenderMain, i);
		INSTANCE.m_workerThreads[i] = std::move(worker);
	}

	INSTANCE.m_isPooled = true;
}

const int thread::RenderThreadHandler::Launch(const int& amount_of_objects)
{
	const unsigned int objects_per_thread = (unsigned int)ceil(amount_of_objects / INSTANCE.m_amount) + 1;
	if (objects_per_thread >= thread::threshold)
	{
		// Launch Threads
		if (INSTANCE.m_isPooled)
		{

			for (unsigned int i = 0; i < INSTANCE.m_amount; i++)
			{
				// Prepare job for threads.
				const auto& f = [=](void* buffer, void* context, void* pipe)
				{
					const int start = i * objects_per_thread;
					const int stop = (i + 1) * objects_per_thread;
					RenderJob(start, stop, buffer, context, pipe);
				};

				INSTANCE.m_jobs.push_back(f);
			}
			cv.notify_all();
		}

		return 0;
	}

	return 1;
}

const bool thread::RenderThreadHandler::GetHandlerStatus()
{
	return INSTANCE.m_isRunning;
}

void thread::RenderThreadHandler::UpdateStatus(const unsigned int& id, unsigned int status)
{
	INSTANCE.m_statuses[id] = status;
}

const unsigned int thread::RenderThreadHandler::GetAmountOfJobs()
{
	return (unsigned int)INSTANCE.m_jobs.size();
}

void thread::RenderThreadHandler::SetRenderer(Renderer* rend)
{
	INSTANCE.m_renderer = rend;
}

Renderer* thread::RenderThreadHandler::GetRenderer()
{
	return INSTANCE.m_renderer;
}

void thread::RenderThreadHandler::SetWindow(Window* wind)
{
	INSTANCE.m_window = wind;
}

Window* thread::RenderThreadHandler::GetWindow()
{
	return INSTANCE.m_window;
}

void thread::RenderThreadHandler::InsertCommandList(ID3D11CommandList* list)
{
	INSTANCE.m_commands.push_back(list);
}

void thread::RenderThreadHandler::ExecuteCommandLists()
{

	// Join Threads
	INSTANCE.Finish();

	// Execute the commands.
	for (int i = 0; i < INSTANCE.m_commands.size(); i++)
	{
		if (INSTANCE.m_commands[i])
		{
			//int j = INSTANCE.m_commands.size();
			D3D11Core::Get().DeviceContext()->ExecuteCommandList(INSTANCE.m_commands[i], true);
			INSTANCE.m_commands[i]->Release();
			INSTANCE.m_commands[i] = nullptr;
		}

	}

	// Remove all traces of evidence.
	INSTANCE.m_commands.clear();
}

void thread::RenderThreadHandler::SetObjectsBuffer(void* objects)
{
	INSTANCE.m_objects = objects;
}

void* thread::RenderThreadHandler::GetObjectsBuffer()
{
	return INSTANCE.m_objects;
}

bool ShouldContinue()
{
	if (!INSTANCE.GetHandlerStatus())
		return true;

	if (INSTANCE.GetAmountOfJobs() > 0)
		return true;

	return false;
}

void RenderMain(const unsigned int& id)
{
	// On start
	PipelineManager pipeManager;
	ID3D11DeviceContext* deferred_context = nullptr;
	dx::ConstantBuffer<basic_model_matrix_t> m_privateBuffer;
	unsigned int t_id = id;
	std::unique_lock<std::mutex> uqmtx(render_mutex);

	// Create deferred context
	D3D11Core::Get().CreateDeferredContext(&deferred_context);
	if (!deferred_context)
	{
		thread::RenderThreadHandler::UpdateStatus(t_id, thread::thread_done);
		return;
	}

	// Setup pipeline.
	pipeManager.Initialize(INSTANCE.GetWindow(), deferred_context);
	m_privateBuffer.Create(D3D11Core::Get().Device());
	thread::RenderThreadHandler::UpdateStatus(t_id, thread::thread_running);

	// On Update
	while (INSTANCE.GetHandlerStatus())
	{
		cv.wait(uqmtx, ShouldContinue);
		if (!INSTANCE.GetHandlerStatus())
		{
			cv.notify_all();
			break;
		}

		EnterCriticalSection(&criticalSection);
		//std::cout << "I entered the critical section! ID: " << t_id << "\n";
		thread::RenderThreadHandler::Get().UpdateStatus(t_id, thread::thread_working);
		// Look for job.
		std::function<void(void*, void*, void*)> func = thread::RenderThreadHandler::Get().GetJob();
		if (func)
		{
			thread::RenderThreadHandler::Get().PopJob();
			LeaveCriticalSection(&criticalSection);
			//std::cout << "I left the critical section! ID: " << t_id << "\n";

			// Run render.
			func(&m_privateBuffer, deferred_context, &pipeManager);
			thread::RenderThreadHandler::Get().UpdateStatus(t_id, thread::thread_running);

			continue;
		}
		LeaveCriticalSection(&criticalSection);
		//std::cout << "I left the critical section! ID: " << t_id << "\n";
		thread::RenderThreadHandler::Get().UpdateStatus(t_id, thread::thread_running);
	}
	thread::RenderThreadHandler::UpdateStatus(t_id, thread::thread_done);

	// On End of Life
	deferred_context->Release();
}

void RenderJob(const unsigned int start,
	unsigned int stop, void* buffer, void* context, void* pipe)
{
	DoubleBuffer<std::vector<comp::Renderable>>* m_objects = (DoubleBuffer<std::vector<comp::Renderable>>*)thread::RenderThreadHandler::GetObjectsBuffer();
	dx::ConstantBuffer<basic_model_matrix_t>* m_buffer = (dx::ConstantBuffer<basic_model_matrix_t>*)buffer;
	ID3D11DeviceContext* m_context = (ID3D11DeviceContext*)context;
	PipelineManager* m_pipeManager = (PipelineManager*)pipe;
	if (m_objects)
	{
		// On Render Start
		ID3D11CommandList* command_list = nullptr;
		IRenderPass* pass = thread::RenderThreadHandler::Get().GetRenderer()->GetCurrentPass();

		pass->PreRender(m_context, m_pipeManager);

		
		// Make sure not to go out of range
		if (stop > (*m_objects)[1].size())
			stop = (unsigned int)(*m_objects)[1].size();
		
		// On Render
		for (unsigned int i = start; i < stop; i++)
		{
			comp::Renderable* it = &(*m_objects)[1][i];
			if (it)
			{
				ID3D11Buffer* buffers[1] =
				{
					m_buffer->GetBuffer()
				};

				m_buffer->SetData(m_context, it->data);
				m_context->VSSetConstantBuffers(0, 1, buffers);
				it->model->RenderDeferred(m_context);
			}
		}

		// On Render Finish
		pass->PostRender(m_context);

		HRESULT hr = m_context->FinishCommandList(true, &command_list);

		EnterCriticalSection(&criticalSection);
		if (SUCCEEDED(hr))
			thread::RenderThreadHandler::Get().InsertCommandList(command_list);
		LeaveCriticalSection(&criticalSection);
	}

	return;
}