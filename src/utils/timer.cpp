#include <iostream>
#include "../utils/timer.h"

JVR_NAMESPACE_OPEN_SCOPE

void Timer::Init(const char* name, size_t n, const char** names)
{
  _name = name;
  _n = n;
  _c = 0;
  _timers.resize(_n, { 0,0,0 });
  _accums.resize(_n, 0.0);
  _avgs.resize(_n, 0.0);
  _names.resize(_n);
  for (size_t t = 0; t < _n; ++t)
    _names[t] = names[t];
}

void Timer::Start(size_t index)
{
  _c = index;
  _timers[_c].Start();
  _rec = true;
}

void Timer::Next()
{
  if (_rec)_timers[_c++].End();
  if (_c >= _n)_c = 0;
  _timers[_c].Start();
}

void Timer::Stop()
{
  if (_rec = true) _timers[_c].End();
  _rec = false;
}

void Timer::Update() {
  for (size_t t = 0; t < _n; ++t) {
    _accums[t] += _timers[t].Elapsed();
    _avgs[t] = (_avgs[t] + _timers[t].Average()) / 2.0;
    _timers[t].Reset();
  }
}

void Timer::Log() {
  std::cout << "------------------   time :   " << _name << std::endl;
  for (size_t t = 0; t < _n; ++t) {
    std::cout << "## " << _names[t] << ": ";
    std::cout << "  - accum : " << _accums[t];
    std::cout << "  - avg : " << _avgs[t] << std::endl;
  }
}

JVR_NAMESPACE_CLOSE_SCOPE
