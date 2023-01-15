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

  typedef int (*TaskFn)(void* data);

  struct Task {
    TaskFn      fn;
    void*       data;
    int Execute() { return fn(data); };
  };

  struct Job {
    std::vector<Task>               tasks;
    std::vector<int>                states;

    Job(TaskFn fn, size_t n, void** datas);

    bool HasPending(size_t* taskIdx=NULL);
    Task* GetPending(size_t taskIdx);
  };

  void Init();

  void AddJob(TaskFn fn, size_t n, void** datas);
  void PopJob();

  bool HasPending();
  Job* GetPending();

private:
  std::mutex                    _mutex;
  std::condition_variable       _waiter;
  std::vector<std::thread>      _workers;
  Job*                          _job;
};


JVR_NAMESPACE_CLOSE_SCOPE


#endif // JVR_ACCELERATION_THREAD_POOL_H
