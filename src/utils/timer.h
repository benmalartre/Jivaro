#ifndef JVR_UTILS_TIMER_H
#define JVR_UTILS_TIMER_H

#if defined(__linux)
	#define HAVE_POSIX_TIMER
	#include <time.h>
	#ifdef CLOCK_MONOTONIC
		#define CLOCKID CLOCK_MONOTONIC
	#else
		#define CLOCKID CLOCK_REALTIME
	#endif
#elif defined(__APPLE__)
	#define HAVE_MACH_TIMER
	#include <mach/mach_time.h>
#elif defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

JVR_NAMESPACE_OPEN_SCOPE

static uint64_t CurrentTime() {
  static uint64_t is_init = 0;
#if defined(__APPLE__)
    static mach_timebase_info_data_t info;
    if (0 == is_init) {
      mach_timebase_info(&info);
      is_init = 1;
    }
    uint64_t now;
    now = mach_absolute_time();
    now *= info.numer;
    now /= info.denom;
    return now;
#elif defined(__linux)
    static struct timespec linux_rate;
    if (0 == is_init) {
      clock_getres(CLOCKID, &linux_rate);
      is_init = 1;
    }
    uint64_t now;
    struct timespec spec;
    clock_gettime(CLOCKID, &spec);
    now = spec.tv_sec * 1.0e9 + spec.tv_nsec;
    return now;
#elif defined(_WIN32)
    static LARGE_INTEGER win_frequency;
    if (0 == is_init) {
      QueryPerformanceFrequency(&win_frequency);
      is_init = 1;
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (uint64_t) ((1e9 * now.QuadPart)  / win_frequency.QuadPart);
#else
  return 0;
#endif
}

// Helpers for benchmark time inside the solver
class Timer {
public:
  void Init(const char* name, size_t n, const char** names);
  void Start(size_t index = 0);
  void Next();
  void Stop();
  void Update();
  void Log();

protected:
  struct _Ts {
    void Start() { t = CurrentTime(); }
    void End() { accum += CurrentTime() - t; num++; };
    void Reset() { accum = 0; num = 0; };
    double Average() { return num ? ((double)accum * 1e-9) / (double)num : 0; }
    double Elapsed() { return (double)accum * 1e-9; }

    uint64_t t;
    uint64_t accum;
    size_t   num;
  };

private:
  bool                      _rec;
  size_t                    _n;
  size_t                    _c;

  std::string               _name;
  std::vector<std::string>  _names;
  std::vector<_Ts>           _timers;
  std::vector<double>       _accums;
  std::vector<double>       _avgs;
};


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

#endif // JVR_UTILS_TIMER_H