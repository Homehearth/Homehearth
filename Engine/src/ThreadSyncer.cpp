#include "EnginePCH.h"
#include "ThreadSyncer.h"

using namespace thread;
#define INSTANCE ThreadSyncer::instance
ThreadSyncer* INSTANCE = nullptr;

ThreadSyncer::ThreadSyncer()
{

}

ThreadSyncer::~ThreadSyncer()
{

}

void ThreadSyncer::Initialize()
{
	if (!INSTANCE)
	{
		INSTANCE = new ThreadSyncer();
	}
}

void ThreadSyncer::Destroy()
{
	if (INSTANCE)
		delete INSTANCE;
}
