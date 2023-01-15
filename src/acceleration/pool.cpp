#include "../acceleration/pool.h"

JVR_NAMESPACE_OPEN_SCOPE

void
WorkerThread(ThreadPool* pool)
{
  while (true) {
    ThreadPool::Task* task = pool->GetPending();
    if(task) {
      task->Execute();
      pool->Signal();
    }
  }
}

ThreadPool::Task*
ThreadPool::GetPending()
{
  std::lock_guard<std::mutex> lock(_mutex);
  size_t numTasks = _tasks.size();
  if (!numTasks || _pending >= numTasks)return NULL;
  return &_tasks[_pending++];
}

void
ThreadPool::Init()
{
  int n = std::thread::hardware_concurrency();
  for (size_t i = 0; i < (n - 1); ++i) {
    _workers.push_back(std::thread(WorkerThread, this));
  }
}

void 
ThreadPool::Signal()
{
  _done++;
}

void 
ThreadPool::BeginTasks()
{
  _done = 0;
  _pending = 0;
  _tasks.clear();
  _mutex.lock();
}

void 
ThreadPool::EndTasks()
{
  _mutex.unlock();
  while (_done < _tasks.size()) {};
}

void
ThreadPool::AddTask(TaskFn fn, ThreadPool::TaskData* data)
{
  _tasks.push_back({ fn, data });
}


JVR_NAMESPACE_CLOSE_SCOPE