#include "../acceleration/pool.h"

JVR_NAMESPACE_OPEN_SCOPE

void
WorkerThread(ThreadPool* pool)
{
  while (true) {
    //pool->WaitTask();

    if (pool->HasPending()) {
      pool->ExecutePending();
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
}

ThreadPool::Job::Job(TaskFn fn, size_t n, ThreadPool::TaskData** datas)
  : cnt(0), n(n)
{
  tasks.resize(n);
  for (size_t t = 0; t < n; ++t) {
    tasks[t] = { fn, datas[t] };
  }
}

bool
ThreadPool::Job::Done()
{
  return cnt == n;
}

bool
ThreadPool::Job::HasPending()
{
  std::lock_guard<std::mutex> lock(mutex);
  return cnt < n;
}

ThreadPool::Task*
ThreadPool::Job::GetPending()
{
  std::lock_guard<std::mutex> lock(mutex);
  return  &tasks[cnt++];
}


void
ThreadPool::ExecutePending()
{
  static ThreadPool::Task task;
  if (!_job || _job->tasks.empty())return;
  _job->GetPending()->Execute();
}


void
ThreadPool::SetPending(Job* job)
{
  _job = job;
}


void
ThreadPool::WaitTask()
{
  std::unique_lock<std::mutex> lock(_mutex);
  _waiter.wait(lock, [this] { return !_job || !_job->tasks.empty(); });
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
  std::cout << "add job..." << std::endl;
  ThreadPool::Job job(fn, n, datas);
  SetPending(&job);
  std::cout << "job : " << _job << std::endl;

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

JVR_NAMESPACE_CLOSE_SCOPE