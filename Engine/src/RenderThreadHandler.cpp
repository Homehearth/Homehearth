#include "EnginePCH.h"
#include "RenderThreadHandler.h"
#include "multi_thread_manager.h"

#define INSTANCE thread::RenderThreadHandler::Get()
#define CONTEXT D3D11Core::Get().DeviceContext()

CRITICAL_SECTION criticalSection;
std::condition_variable cv;
std::mutex render_mutex;
std::mutex thread_mutex;
bool shouldRender = true;

bool ShouldContinue();
void RenderMain(const unsigned int& id);
void RenderJob(const unsigned int start, const unsigned int stop, void* objects, void* buffer, void* context);

thread::RenderThreadHandler::RenderThreadHandler()
{
	m_workerThreads = nullptr;
	m_amount = 0;
	m_statuses = nullptr;
	m_isRunning = false;
	InitializeCriticalSection(&criticalSection);
}

thread::RenderThreadHandler::~RenderThreadHandler()
{
	INSTANCE.m_isRunning = false;
	cv.notify_all();
	if (m_workerThreads)
	{
		for (int i = 0; i < INSTANCE.m_amount; i++)
		{
			if (INSTANCE.m_workerThreads[i].joinable())
				INSTANCE.m_workerThreads[i].join();
			else
				INSTANCE.m_workerThreads[i].detach();
		}
		delete[] INSTANCE.m_workerThreads;
		delete[] INSTANCE.m_statuses;
	}
}

void thread::RenderThreadHandler::Finish()
{
	if (!INSTANCE.m_isPooled)
	{
		for (int i = 0; i < INSTANCE.m_amount; i++)
		{
			INSTANCE.m_workerThreads[i].join();
		}
	}
	else
	{
		// Block until all threads have stopped working.
		for (int i = 0; i < INSTANCE.m_amount; i++)
		{
			if (INSTANCE.GetStatus(i) != thread::thread_running)
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

void thread::RenderThreadHandler::InsertRenderJob(std::function<void(void*, void*)> job)
{
	INSTANCE.Get().m_jobs.push_back(job);
}

std::function<void(void*, void*)> thread::RenderThreadHandler::GetJob()
{
	if (INSTANCE.m_jobs.size() <= 0)
	{
		return nullptr;
	}

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
		std::thread worker(RenderMain, i);
		INSTANCE.m_workerThreads[i] = std::move(worker);
	}

	INSTANCE.m_isPooled = true;
}

const int thread::RenderThreadHandler::Launch(const int& amount_of_objects, void* objects)
{
	unsigned int objects_per_thread = (amount_of_objects / INSTANCE.m_amount);
	if (objects_per_thread >= thread::threshold)
	{
		// Convert 
		DoubleBuffer<std::vector<comp::Renderable>>* m_objects = (DoubleBuffer<std::vector<comp::Renderable>>*)objects;

		unsigned int iterations = (amount_of_objects / objects_per_thread);

		// Launch Threads
		if (INSTANCE.m_isPooled)
		{
			for (int i = 0; i < iterations; i++)
			{
				auto f = [=](void* buffer, void* context)
				{
					RenderJob(i * objects_per_thread, (i + 1) * objects_per_thread, m_objects, buffer, context);
				};
				INSTANCE.m_jobs.push_back(f);
			}
			cv.notify_all();
		}

		// Join Threads
		INSTANCE.Finish();

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
	ID3D11DeviceContext* deferred_context = nullptr;
	dx::ConstantBuffer<basic_model_matrix_t> m_privateBuffer;
	unsigned int t_id = id;
	std::unique_lock<std::mutex> uqmtx(render_mutex);

	D3D11Core::Get().CreateDeferredContext(&deferred_context);
	if (!deferred_context)
	{
		thread::RenderThreadHandler::UpdateStatus(t_id, thread::thread_done);
		return;
	}
	m_privateBuffer.Create(D3D11Core::Get().Device());
	thread::RenderThreadHandler::UpdateStatus(t_id, thread::thread_running);

	// On Update
	while (INSTANCE.GetHandlerStatus())
	{
		cv.wait(uqmtx, ShouldContinue);
		EnterCriticalSection(&criticalSection);
		if (!INSTANCE.GetHandlerStatus())
		{
			LeaveCriticalSection(&criticalSection);
			cv.notify_all();
			break;
		}

		// Look for job.
		std::function<void(void*, void*)> func = thread::RenderThreadHandler::Get().GetJob();
		if (func)
		{
			thread::RenderThreadHandler::Get().UpdateStatus(t_id, thread::thread_working);
			thread::RenderThreadHandler::Get().PopJob();
			LeaveCriticalSection(&criticalSection);

			// Run render.
			func(&m_privateBuffer, deferred_context);
			thread::RenderThreadHandler::Get().UpdateStatus(t_id, thread::thread_running);

			continue;
		}
		LeaveCriticalSection(&criticalSection);
	}
	thread::RenderThreadHandler::UpdateStatus(t_id, thread::thread_done);

	// On End of Life
	deferred_context->Release();
}

void RenderJob(const unsigned int start,
	const unsigned int stop, void* objects, void* buffer, void* context)
{
	DoubleBuffer<std::vector<comp::Renderable>>* m_objects = (DoubleBuffer<std::vector<comp::Renderable>>*)objects;
	dx::ConstantBuffer<basic_model_matrix_t>* m_buffer = (dx::ConstantBuffer<basic_model_matrix_t>*)buffer;
	ID3D11DeviceContext* m_context = (ID3D11DeviceContext*)context;
	if (m_objects)
	{

		// Update Context


		for (int i = start; i < stop; i++)
		{
			comp::Renderable* it = &(*m_objects)[1].at(i);
			if (it)
			{
				ID3D11Buffer* buffers[1] =
				{
					m_buffer->GetBuffer()
				};

				m_buffer->SetData(m_context, it->data);
				m_context->VSSetConstantBuffers(0, 1, buffers);
				it->mesh->RenderDeferred(m_context);
			}
		}

		// Release Context

	}

}