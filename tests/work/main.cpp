#include <iostream>


#include <pxr/pxr.h>
#include <pxr/base/tf/stopwatch.h>
#include <pxr/base/work/loops.h>
#include <pxr/base/gf/vec4i.h>

#include "../../src/acceleration/pool.h"

JVR_NAMESPACE_OPEN_SCOPE

struct TaskData {
  pxr::GfVec4i* values;
  size_t        start;
  size_t        end;
};

void _Init(std::vector<pxr::GfVec4i>& values)
{
  for(size_t i = 0; i < values.size(); ++i) {
    values[i] = pxr::GfVec4i(i%10, 0, 0, 1);
  }
}

bool _Check(std::vector<pxr::GfVec4i>& values)
{
  for(size_t i = 0; i < values.size(); ++i) {
    if(values[i] != pxr::GfVec4i(0, 0, i%10, 1))return false;
  }
  return true;
}

void _Permute(size_t start, size_t end, pxr::GfVec4i* values)
{
  for(size_t index = start; index < end; ++index) {
    int tmp = values[index][2];
    values[index][2] = values[index][0];
    values[index][0] = tmp;
  }
}

void _Permute2(void* datas)
{
  TaskData* _datas = (TaskData*)datas;
  for(size_t index = _datas->start; index < _datas->end; ++index) {
    int tmp = _datas->values[index][2];
    _datas->values[index][2] = _datas->values[index][0];
    _datas->values[index][0] = tmp;
  }
}

JVR_NAMESPACE_CLOSE_SCOPE

JVR_NAMESPACE_USING_DIRECTIVE

int main(void)
{
  size_t numTasks = 64;
  size_t numElements = 10000000;
  ThreadPool pool;
  pool.Init();

  std::vector<pxr::GfVec4i> datas(numElements);

  pxr::TfStopwatch sw;
  
  _Init(datas);
  sw.Start();

  _Permute(0, numElements, &datas[0]);
  sw.Stop();

  std::cout << "SERIAL tooks " << sw.GetSeconds() << " seconds for " << numTasks << " elements" << std::endl;
  if(!_Check(datas))
    std::cout << "SERIAL FAIL" << std::endl;
  else 
  std::cout << "SERIAL SUCCESS" << std::endl;
  
  _Init(datas);
  sw.Reset();
  sw.Start();

  pxr::WorkParallelForN(
    numElements, 
    std::bind(&_Permute, std::placeholders::_1, std::placeholders::_2, &datas[0]),
      numTasks);
  sw.Stop();
  std::cout << "TBB tooks " << sw.GetSeconds() << " seconds for " << numTasks << " elements" << std::endl;
  if(!_Check(datas))
    std::cout << "TBB FAIL" << std::endl;
  else 
  std::cout << "TBB SUCCESS" << std::endl;

  _Init(datas);
  std::vector<TaskData> threadDatas(numTasks);

  sw.Reset();
  sw.Start();

  pool.BeginTasks();

  size_t chunkSize = (numElements + numTasks - (numElements % numTasks)) / numTasks;
  for (size_t t = 0; t < numTasks; ++t) {
    threadDatas[t].values = &datas[0];
    threadDatas[t].start = t * chunkSize;
    threadDatas[t].end = std::min((t + 1) * chunkSize, numElements);
    pool.AddTask(_Permute2, (void*)&threadDatas[t]);
  }
  pool.EndTasks();

  sw.Stop();
  std::cout << "POOL tooks " << sw.GetSeconds() << " seconds for " << numTasks << " elements" << std::endl;
  if(!_Check(datas))
    std::cout << "POOL FAIL" << std::endl;
  else 
  std::cout << "POOL SUCCESS" << std::endl;
  pool.Term();

  return 1;
}