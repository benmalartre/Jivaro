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
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
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

bool 
ThreadPool::Done()
{
  std::lock_guard<std::mutex> lock(_mutex);
  if (_tasks.size() == _done) {
    _tasks.clear();
    _pending = 0;
    _done = 0;
    return true;
  }
  return false;
}

void 
ThreadPool::BeginTasks()
{
  _done = 0;
  _pending = 0;
  _tasks.clear();
}

void 
ThreadPool::EndTasks()
{
  while (_done < _tasks.size()) {};
}

void
ThreadPool::AddTask(TaskFn fn, ThreadPool::TaskData* data)
{
  std::lock_guard<std::mutex> lock(_mutex);
  _tasks.push_back({ fn, data });
}


JVR_NAMESPACE_CLOSE_SCOPE