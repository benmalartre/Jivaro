#ifndef JVR_UTILS_TIMER_H
#define JVR_UTILS_TIMER_H

#include <vector>
#include <pxr/base/arch/timing.h>
#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE


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
    void Start() { t = ArchGetTickTime(); }
    void End() { accum += ArchGetTickTime() - t; num++; };
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