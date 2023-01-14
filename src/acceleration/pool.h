#ifndef JVR_ACCELERATION_THREAD_POOL_H
#define JVR_ACCELERATION_THREAD_POOL_H

#include "../common.h"
#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>


JVR_NAMESPACE_OPEN_SCOPE

class ThreadPool {

public:
  enum State {
    WAITING,
    WORKING,
    DONE
  };

  typedef int (*ThreadFn)(void* data);

  struct Thread {
    ThreadFn    fn;
    void*       data;
    int Execute() { return fn(data); };
  };

  struct Task {
    std::vector<Thread>             threads;
    std::vector<int>                states;

    Task(ThreadFn fn, std::vector<void*>& datas);

    bool HasPending(size_t* threadIdx=NULL);
    Thread* GetPending(size_t threadIdx);

  };

  void WorkerThread(ThreadPool* pool, size_t threadIdx)
  {
    while (true) { 
      // wait until some task
      if (!pool->HasTask()) {
        pool->SetState(threadIdx, ThreadPool::WAITING);
      } else {
        ThreadPool::Task* task = pool->GetTask();
        size_t threadIdx;
        if (task->HasPending(&threadIdx)) {
          Thread* thread = task->GetPending(threadIdx);
          pool->SetState(threadIdx, ThreadPool::WORKING);
          thread->Execute();
          pool->SetState(threadIdx, ThreadPool::DONE);
        }
        else {
          pool->PopTask();
        }
      }
    }
  }

  //unsigned int nthreads = std::thread::hardware_concurrency();
  void SetNumThreads(size_t numThreads);
  size_t GetNumThreads();

  void AddTask(ThreadFn fn, std::vector<void*>& datas);
  void PopTask();

  bool HasTask();
  Task* GetTask();
  std::mutex& GetMutex() { return _mutex; };
  void SetState(size_t threadIdx, ThreadPool::State state);

private:
  std::mutex                            _mutex;
  std::vector<std::thread>              _workers;
  std::vector<std::condition_variable>  _waiters;
  std::queue<Task>                      _tasks;
};

JVR_NAMESPACE_CLOSE_SCOPE


#endif // JVR_ACCELERATION_THREAD_POOL_H
