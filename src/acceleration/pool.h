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

  typedef int (*TaskFn)(TaskData* data);

  struct Task {
    TaskFn      fn;
    TaskData*   data;
    int Execute() { return fn(data); };
  };

  struct Job {
    unsigned int            n;
    unsigned int            cnt;
    std::condition_variable waiter;
    std::mutex              mutex;
    std::vector<Task>       tasks;

    Job(TaskFn fn, size_t n, TaskData** datas);

    bool Done();
    bool HasPending();
    Task* GetPending();
  };

  void Init();

  void AddJob(TaskFn fn, size_t n, TaskData** datas);
  bool HasPending();
  void SetPending(Job* job);
  void ExecutePending();
  void WaitTask();

private:
  std::mutex                    _mutex;
  std::condition_variable       _waiter;
  std::vector<std::thread>      _workers;
  Job*                          _job;
};


JVR_NAMESPACE_CLOSE_SCOPE


#endif // JVR_ACCELERATION_THREAD_POOL_H
