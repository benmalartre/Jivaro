#include "../acceleration/pool.h"

JVR_NAMESPACE_OPEN_SCOPE

static void
WorkerThread(ThreadPool* pool)
{
  while (true) {
    if (pool->HasPending()) {
      ThreadPool::Job* job = pool->GetPending();
      size_t taskIdx;
      if (job->HasPending(&taskIdx)) {
        ThreadPool::Task* task = job->GetPending(taskIdx);
        job->states[taskIdx] = ThreadPool::WORKING;
        task->Execute();
        job->states[taskIdx] = ThreadPool::DONE;
      }
    }
    else {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
}

ThreadPool::Job::Job(TaskFn fn, size_t n, ThreadPool::TaskData** datas)
{
  states.resize(n);
  for (size_t t = 0; t < n; ++t) {
    tasks.push({ fn, datas[t] });
    states[t] = ThreadPool::WAITING;
  }
}

bool
ThreadPool::Job::Done()
{
  for (size_t i = 0; i < states.size(); ++i) {
    if (states[i] != ThreadPool::DONE) {
      return false;
    }
  }
  return true;
}

bool
ThreadPool::Job::HasPending()
{
  return tasks.size() > 0;
}

ThreadPool::Task*
ThreadPool::Job::GetPending()
{
  size_t taskIdx = states.size() - (tasks.size());
  std::lock_guard<std::mutex> lock(mutex);
  states[taskIdx] = ThreadPool::WORKING;
  return tasks.pop();
}

void
ThreadPool::SetPending(Job* job)
{
  std::lock_guard<std::mutex> lock(_mutex);
  _job = job;
}

void
ThreadPool::Init()
{
  _job = NULL;
  int n = std::thread::hardware_concurrency();
  for (size_t i = 0; i < (n - 1); ++i) {
    _workers.push_back(std::thread(WorkerThread, this));
  }
}

void
ThreadPool::AddJob(TaskFn fn, size_t n, ThreadPool::TaskData** datas)
{
  ThreadPool::Job job(fn, n, datas);
  SetPending(&job);

  while (!job.Done()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  SetPending(NULL);
}



bool 
ThreadPool::HasPending()
{
  if (!_job)return false;
  return true;
}

ThreadPool::Job*
ThreadPool::GetPending()
{
  return _job;
}

JVR_NAMESPACE_CLOSE_SCOPE