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

  struct TaskData {
  };

  typedef void (*TaskFn)(TaskData* data);

  struct Task {
    TaskFn      fn;
    TaskData*   data;
    void Execute() { fn(data); };
  };

  class Semaphore {
  public:
    Semaphore(size_t count=0) : cnt(count) {};

    void Reset(size_t count);
    void Notify();
    void Wait();

  private:
    size_t                  cnt;
    std::mutex              mtx;
    std::condition_variable cv;
  };

  void Init();
  void BeginTasks();
  void AddTask(TaskFn fn, TaskData* datas);
  void EndTasks();
  void Signal();
  Task* GetPending();
  void Wait();

private:
  Semaphore                     _start, _run;
  std::vector<std::thread>      _workers;
  std::vector<Task>             _tasks;
  std::atomic<int>              _pending;
  std::atomic<int>              _done;
  
};


JVR_NAMESPACE_CLOSE_SCOPE


#endif // JVR_ACCELERATION_THREAD_POOL_H
