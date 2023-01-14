#include "../acceleration/pool.h"

JVR_NAMESPACE_OPEN_SCOPE

ThreadPool::Task::Task(ThreadFn fn, std::vector<void*>& datas)
{
     threads.resize(datas.size());
     states.resize(datas.size());
     for (size_t t = 0; t < datas.size(); ++t) {
       threads[t].fn = fn;
       threads[t].data = datas[t];
     }
}

bool
ThreadPool::Task::HasPending(size_t* threadIdx)
{
  for (size_t i = 0; i < states.size(); ++i) {
    if (states[i] == ThreadPool::WAITING) {
      if(threadIdx)*threadIdx = i; 
      return true;
    }
  }
  return false;
}

ThreadPool::Thread*
ThreadPool::Task::GetPending(size_t threadIdx)
{
  states[threadIdx] = ThreadPool::WORKING;
  return &threads[threadIdx];
}

void 
ThreadPool::SetNumThreads(size_t numThreads)
{

}

size_t 
ThreadPool::GetNumThreads() 
{ 
  return _workers.size(); 
};

void 
ThreadPool::SetState(size_t threadIdx, ThreadPool::State state)
{
  std::unique_lock<std::mutex> lock(_mutex);
  switch(state) {
  case ThreadPool::WAITING:
    _waiters[threadIdx].wait(lock, [&] { return HasTask(); });
    break;
  default:
    _waiters[threadIdx].notify_all();
    break;
  }
}

void
ThreadPool::AddTask(ThreadFn fn, std::vector<void*>& datas)
{
  const std::lock_guard<std::mutex> lock(_mutex);
  _tasks.push(Task(fn, datas));
}

void
ThreadPool::PopTask()
{
  const std::lock_guard<std::mutex> lock(_mutex);
  _tasks.pop();
}


bool 
ThreadPool::HasTask()
{
  if (!_tasks.size())return false;
  if (!_tasks.front().HasPending()) {
    PopTask();
    return HasTask();
  }
  return true;
}

ThreadPool::Task*
ThreadPool::GetTask()
{
  if (!_tasks.size())return NULL;
  const std::lock_guard<std::mutex> lock(_mutex);
  return &_tasks.front();
}

JVR_NAMESPACE_CLOSE_SCOPE