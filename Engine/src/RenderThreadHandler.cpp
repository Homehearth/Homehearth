#include "EnginePCH.h"
#include "RenderThreadHandler.h"
#include "multi_thread_manager.h"
#include "ShadowPass.h"
#include <omp.h>

#define INSTANCE thread::RenderThreadHandler::Get()
#define CONTEXT D3D11Core::Get().DeviceContext()

CRITICAL_SECTION criticalSection;
CRITICAL_SECTION pushSection;
std::condition_variable cv;
bool shouldRender = true;

bool ShouldContinue();
void RenderMain(const unsigned int id);
void RenderJob(const unsigned int start, unsigned int stop, void* buffer, void* context);
void RenderShadow(const unsigned int start, unsigned int stop, void* buffer, void* context);

thread::RenderThreadHandler::RenderThreadHandler()
{
	m_workerThreads = nullptr;
	m_amount = 0;
	m_camera = nullptr;
	m_renderer = nullptr;
	m_window = nullptr;
	m_statuses = nullptr;
	m_objects = nullptr;
	m_isRunning = false;
	m_isActive = false;
	InitializeCriticalSection(&criticalSection);
	InitializeCriticalSection(&pushSection);
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
	if (INSTANCE.m_isActive)
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
			while ((int)INSTANCE.m_commands.size() != INSTANCE.m_amount) {
			};
		}
	}
	else
		return;
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

const render_instructions_t thread::RenderThreadHandler::Launch(const int& amount_of_objects)
{
	render_instructions_t inst;
	const unsigned int objects_per_thread = (unsigned int)std::floor((float)amount_of_objects / (float)(INSTANCE.m_amount + 1));
	int main_start = 0;
	if (objects_per_thread >= thread::BASE_THRESHOLD && amount_of_objects >= (int)(INSTANCE.m_amount + 1))
	{
		// Launch Threads
		if (INSTANCE.m_isPooled)
		{
			for (unsigned int i = 0; i < INSTANCE.m_amount; i++)
			{
				const int start = i * objects_per_thread;
				const int stop = start + objects_per_thread;
				// Prepare job for threads.
				auto f = [=](void* buffer, void* context)
				{
					RenderJob(start, stop, buffer, context);
				};

				INSTANCE.m_jobs.push_back(f);
				main_start = stop;
			}
			cv.notify_all();
		}

		inst.start = main_start;
		inst.stop = amount_of_objects;
		INSTANCE.m_isActive = true;
		return inst;
	}

	INSTANCE.m_isActive = false;
	return inst;
}

const render_instructions_t thread::RenderThreadHandler::DoShadows(const int& amount_of_objects)
{
	render_instructions_t inst;
	const unsigned int objects_per_thread = (unsigned int)std::floor((float)amount_of_objects / (float)(INSTANCE.m_amount + 1));
	int main_start = 0;
	if (objects_per_thread >= thread::SHADOW_THRESHOLD && amount_of_objects >= (int)(INSTANCE.m_amount + 1))
	{
		// Launch Threads
		if (INSTANCE.m_isPooled)
		{
			for (unsigned int i = 0; i < INSTANCE.m_amount; i++)
			{
				const int start = i * objects_per_thread;
				const int stop = start + objects_per_thread;
				// Prepare job for threads.
				auto f = [=](void* buffer, void* context)
				{
					RenderShadow(start, stop, buffer, context);
				};

				INSTANCE.m_jobs.push_back(f);
				main_start = stop;
			}
			cv.notify_all();
		}

		inst.start = main_start;
		inst.stop = amount_of_objects;
		INSTANCE.m_isActive = true;
		return inst;
	}

	INSTANCE.m_isActive = false;
	return inst;
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
			int j = (int)INSTANCE.m_commands.size();
#if RENDER_IMGUI
			D3D11Core::Get().DeviceContext()->ExecuteCommandList(INSTANCE.m_commands[i], 1);
#else
			D3D11Core::Get().DeviceContext()->ExecuteCommandList(INSTANCE.m_commands[i], 0);
#endif
			INSTANCE.m_commands[i]->Release();
			INSTANCE.m_commands[i] = nullptr;
		}

	}

	// Remove all traces of evidence.
	INSTANCE.m_commands.clear();
	int j = (int)INSTANCE.m_commands.size();
}

void thread::RenderThreadHandler::SetObjectsBuffer(void* objects)
{
	INSTANCE.m_objects = objects;
}

void* thread::RenderThreadHandler::GetObjectsBuffer()
{
	return INSTANCE.m_objects;
}

void thread::RenderThreadHandler::SetCamera(void* camera)
{
	INSTANCE.m_camera = camera;
}

void* thread::RenderThreadHandler::GetCamera()
{
	return INSTANCE.m_camera;
}

void thread::RenderThreadHandler::SetShadows(void* shadows)
{
	INSTANCE.m_shadows = shadows;
}

void* thread::RenderThreadHandler::GetShadows()
{
	return INSTANCE.m_shadows;
}

bool ShouldContinue()
{
	if (!INSTANCE.GetHandlerStatus())
		return true;

	if (INSTANCE.GetAmountOfJobs() > 0)
		return true;

	return false;
}

void RenderMain(const unsigned int id)
{
	// On start
	ID3D11DeviceContext* deferred_context = nullptr;
	dx::ConstantBuffer<basic_model_matrix_t> m_privateBuffer;
	unsigned int t_id = id;
	std::mutex mtx;
	std::unique_lock<std::mutex> uqmtx(mtx);

	// Create deferred context
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
		// Sleep until called upon.
		cv.wait(uqmtx, ShouldContinue);

		if (!INSTANCE.GetHandlerStatus())
		{
			cv.notify_all();
			break;
		}

		EnterCriticalSection(&criticalSection);
		thread::RenderThreadHandler::Get().UpdateStatus(t_id, thread::thread_working);
		// Look for job.
		std::function<void(void*, void*)> func = thread::RenderThreadHandler::Get().GetJob();
		if (func)
		{
			cv.notify_all();
			thread::RenderThreadHandler::Get().PopJob();
			LeaveCriticalSection(&criticalSection);

			// Run render.
			//double start = omp_get_wtime();
			func(&m_privateBuffer, deferred_context);
			//double end = omp_get_wtime() - start;
			//std::cout << "Preparation for render took: " << end << "\n";
		}
		else
		{
			LeaveCriticalSection(&criticalSection);
		}
		thread::RenderThreadHandler::Get().UpdateStatus(t_id, thread::thread_running);
	}
	thread::RenderThreadHandler::UpdateStatus(t_id, thread::thread_done);

	// On End of Life
	deferred_context->Release();
}

void RenderJob(const unsigned int start,
	unsigned int stop, void* buffer, void* context)
{
	DoubleBuffer<std::vector<comp::Renderable>>* m_objects = (DoubleBuffer<std::vector<comp::Renderable>>*)thread::RenderThreadHandler::GetObjectsBuffer();
	dx::ConstantBuffer<basic_model_matrix_t>* m_buffer = (dx::ConstantBuffer<basic_model_matrix_t>*)buffer;
	ID3D11DeviceContext* m_context = (ID3D11DeviceContext*)context;
	if (m_objects)
	{
		// On Render Start
		ID3D11CommandList* command_list = nullptr;
		IRenderPass* pass = thread::RenderThreadHandler::Get().GetRenderer()->GetCurrentPass();
		Camera* cam = (Camera*)INSTANCE.Get().GetCamera();

		pass->PreRender(cam, m_context);

		// Make sure not to go out of range
		if (stop > (unsigned int)(*m_objects)[1].size())
			stop = (unsigned int)(*m_objects)[1].size();
		
		
		// On Render
		for (unsigned int i = start; i < stop; i++)
		{
			comp::Renderable* it = &(*m_objects)[1][i];
			ID3D11Buffer* const buffers[1] =
			{
				m_buffer->GetBuffer()
			};

			m_buffer->SetData(m_context, it->data);
			m_context->VSSetConstantBuffers(0, 1, buffers);
			
			if (it->model)
				it->model->Render(m_context);

		}

		pass->PostRender(m_context);

		// On Render Finish
		HRESULT hr = m_context->FinishCommandList(0, &command_list);

		EnterCriticalSection(&pushSection);
		if (SUCCEEDED(hr))
			thread::RenderThreadHandler::Get().InsertCommandList(std::move(command_list));
		LeaveCriticalSection(&pushSection);
	}

	return;
}

void RenderShadow(const unsigned int start, unsigned int stop, void* buffer, void* context)
{
	DoubleBuffer<std::vector<comp::Renderable>>* m_objects = (DoubleBuffer<std::vector<comp::Renderable>>*)thread::RenderThreadHandler::GetObjectsBuffer();
	std::vector<ShadowSection>* m_shadows = (std::vector<ShadowSection>*)thread::RenderThreadHandler::Get().GetShadows();
	dx::ConstantBuffer<basic_model_matrix_t>* m_buffer = (dx::ConstantBuffer<basic_model_matrix_t>*)buffer;
	ID3D11DeviceContext* m_context = (ID3D11DeviceContext*)context;
	ShadowPass* currentPass = dynamic_cast<ShadowPass*>(thread::RenderThreadHandler::Get().GetRenderer()->GetCurrentPass());
	ID3D11CommandList* commandList = nullptr;

	if (m_objects && m_shadows)
	{
		// Prerender.
		currentPass->PreRender(nullptr, m_context);

		// For each shadow.
		for (int i = start; i < stop; i++)
		{
			currentPass->UpdateLightBuffer(m_context, (*m_shadows)[i].lightBuffer.Get(), *(*m_shadows)[i].pLight);

			m_context->ClearDepthStencilView((*m_shadows)[i].shadowDepth.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
			ID3D11RenderTargetView* nullTargets[8] = { nullptr };
			m_context->VSSetConstantBuffers(1, 1, (*m_shadows)[i].lightBuffer.GetAddressOf());
			m_context->OMSetRenderTargets(8, nullTargets, (*m_shadows)[i].shadowDepth.Get());
			const light_t* light = (*m_shadows)[i].pLight;
			// For each object in world.
			for (int j = 0; j < (*m_objects)[1].size(); j++)
			{
				comp::Renderable* it = &(*m_objects)[1][j];
				sm::Vector3 translation = it->data.worldMatrix.Translation();
				float distance = (translation - sm::Vector3(light->position)).Length();
				if (distance < light->range && it->isSolid && it->visible)
				{
					ID3D11Buffer* const buffers[1] =
					{
						m_buffer->GetBuffer()
					};

					m_buffer->SetData(m_context, it->data);
					m_context->VSSetConstantBuffers(0, 1, buffers);
					it->model->Render(m_context);
				}
			}

			m_context->OMSetRenderTargets(8, nullTargets, nullptr);
		}


		// Postrender.
		currentPass->PostRender(m_context);

		HRESULT hr = m_context->FinishCommandList(false, &commandList);
		EnterCriticalSection(&pushSection);
		if(SUCCEEDED(hr))
			thread::RenderThreadHandler::Get().InsertCommandList(std::move(commandList));
		LeaveCriticalSection(&pushSection);
	}
}