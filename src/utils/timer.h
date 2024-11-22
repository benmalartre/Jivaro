#ifndef JVR_UTILS_TIMER_H
#define JVR_UTILS_TIMER_H

#include <vector>

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

#include "../common.h"

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
    return now / 1000;
#elif defined(__linux)
    static struct timespec linux_rate;
    if (0 == is_init) {
      clock_getres(CLOCKID, &linux_rate);
      is_init = 1;
    }
    uint64_t now;
    struct timespec spec;
    clock_gettime(CLOCKID, &spec);
    now = spec.tv_sec * 1.0e6 + spec.tv_nsec;
    return now;
#elif defined(_WIN32)
    static LARGE_INTEGER win_frequency;
    static double microseconds_per_count;
    if (0 == is_init) {
      QueryPerformanceFrequency(&win_frequency);
      microseconds_per_count = 1.0e6 / win_frequency.QuadPart;
      is_init = 1;
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (uint64_t) (now.QuadPart * microseconds_per_count);
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
    double Average() { return num ? ((double)accum * 1.0e-6) / (double)num : 0; }
    double Elapsed() { return (double)accum * 1.0e-6; }

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
  std::vector<_Ts>          _timers;
  std::vector<double>       _accums;
  std::vector<double>       _avgs;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UTILS_TIMER_H