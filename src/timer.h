//------------------------------------------------------------------------------------------------
// Easy embeddable cross-platform high resolution timer function. For each
// platform we select the high resolution timer. You can call the 'ns()'
// function in your file after embedding this.
// Borrowed from https://github.com/ahmidou/SoftimageCageDeformers/blob/master/definitions.h
//------------------------------------------------------------------------------------------------
#ifndef _TIMER_H_
#define _TIMER_H_

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

static uint64_t ns() {
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
#endif
}

#endif //_TIMER_H_
