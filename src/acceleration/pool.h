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

  void Init();
  void BeginTasks();
  void AddTask(TaskFn fn, TaskData* datas);
  void EndTasks();
  void Signal();
  Task* GetPending();

private:
  std::condition_variable       _waiter;
  std::mutex                    _mutex;
  std::vector<std::thread>      _workers;
  std::vector<Task>             _tasks;
  std::atomic<int>              _pending;
  std::atomic<int>              _done;
};


JVR_NAMESPACE_CLOSE_SCOPE


#endif // JVR_ACCELERATION_THREAD_POOL_H
