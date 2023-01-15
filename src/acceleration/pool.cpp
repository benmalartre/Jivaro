#include "../acceleration/pool.h"

JVR_NAMESPACE_OPEN_SCOPE

static void
WorkerThread(ThreadPool* pool)
{
  while (true) {
    // wait until some task
    if (pool->HasPending()) {
      ThreadPool::Job* job = pool->GetPending();
      std::cout << "[worker] get job " << job << std::endl;
      size_t taskIdx;
      if (job->HasPending(&taskIdx)) {
        std::cout << "[worker] task id " << taskIdx << std::endl;
        ThreadPool::Task* task = job->GetPending(taskIdx);
        task->Execute();
        job->states[taskIdx] = ThreadPool::DONE;
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
    else {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
}

ThreadPool::Job::Job(TaskFn fn, size_t n, void** datas)
{
  tasks.resize(n);
  states.resize(n);
  for (size_t t = 0; t < n; ++t) {
    tasks[t].fn = fn;
    tasks[t].data = datas[t];
    std::cout << "task data : " << tasks[t].data << std::endl;
    states[t] = ThreadPool::WAITING;
  }
}

bool
ThreadPool::Job::HasPending(size_t* taskIdx)
{
  for (size_t i = 0; i < states.size(); ++i) {
    if (states[i] == ThreadPool::WAITING) {
      std::cout << "[pool] task has pending " << i << std::endl;
      if(taskIdx)*taskIdx = i; 
      return true;
    }
  }
  return false;
}

ThreadPool::Task*
ThreadPool::Job::GetPending(size_t taskIdx)
{
  states[taskIdx] = ThreadPool::WORKING;
  return &tasks[taskIdx];
}

void
ThreadPool::Init()
{
  _job = NULL;
  unsigned int n = std::thread::hardware_concurrency();
  std::cout << "[pool] initialize : " << n << " processors " << std::endl;
  for (size_t i = 0; i < n - 1; ++i) {
    _workers.push_back(std::thread(WorkerThread, this));
  }
  std::cout << "[pool] create worker threads " << n << std::endl;
}

void
ThreadPool::AddJob(TaskFn fn, size_t n, void** datas)
{
  std::lock_guard<std::mutex> lock(_mutex);
  std::cout << " add job " << n << std::endl;
  _job = new Job(fn, n, datas);
  std::cout << "job added : " << _job << std::endl;

  while (_job->HasPending()) {
  }
  std::cout << "job done : " << _job << std::endl;

}

void
ThreadPool::PopJob()
{
  std::lock_guard<std::mutex> lock(_mutex);
  delete _job;
  _job = NULL;
}


bool 
ThreadPool::HasPending()
{
  if (!_job)return false;
  if (!_job->HasPending()) {
    PopJob();
    return false;
  }
  return true;
}

ThreadPool::Job*
ThreadPool::GetPending()
{
  return _job;
}

JVR_NAMESPACE_CLOSE_SCOPE