#include "../acceleration/pool.h"

JVR_NAMESPACE_OPEN_SCOPE

void
WorkerThread(ThreadPool* pool)
{
  while (true) {
    pool->Wait();

    ThreadPool::Task* task = pool->GetPending();
    if(task) {
      task->Execute();
      pool->Signal();
    }
  }
}

void
ThreadPool::Semaphore::Reset(size_t count)
{
  std::unique_lock<std::mutex> lock(mtx);
  cnt = count;
  cv.notify_all();
}


void
ThreadPool::Semaphore::Notify()
{
  std::unique_lock<std::mutex> lock(mtx);
  cnt++;
  cv.notify_one();
}

void
ThreadPool::Semaphore::Wait()
{
  std::unique_lock<std::mutex> lock(mtx);
  while (cnt == 0) {
    cv.wait(lock);
  }
  cnt--;
}

ThreadPool::Task*
ThreadPool::GetPending()
{
  ThreadPool::Task* task = NULL;
  if(_pending < _tasks.size())
    task = &_tasks[_pending++];
  return task;
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
  _run.Notify();
}

void
ThreadPool::Wait()
{
  _start.Wait();
  _run.Wait();
}

void 
ThreadPool::BeginTasks()
{
  _done = 0;
  _pending = 0;
  _tasks.clear();
}

void
ThreadPool::AddTask(TaskFn fn, ThreadPool::TaskData* data)
{
  _tasks.push_back({ fn, data });
  _start.Notify();
}

void 
ThreadPool::EndTasks()
{
  size_t numTasks = _tasks.size();
  _run.Reset(numTasks);
  while (_done < numTasks) {};
}


JVR_NAMESPACE_CLOSE_SCOPE