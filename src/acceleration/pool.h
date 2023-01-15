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
    std::mutex           mutex;
    std::queue<Task>     tasks;
    std::vector<int>     states;

    Job(TaskFn fn, size_t n, TaskData** datas);

    bool Done();
    bool HasPending();
    Task* GetPending();
  };

  void Init();

  void AddJob(TaskFn fn, size_t n, TaskData** datas);
  void PopJob();

  bool HasPending();
  void SetPending(Job* job);
  Job* GetPending();

private:
  std::mutex                    _mutex;
  std::condition_variable       _waiter;
  std::vector<std::thread>      _workers;
  Job*                          _job;
};


JVR_NAMESPACE_CLOSE_SCOPE


#endif // JVR_ACCELERATION_THREAD_POOL_H
