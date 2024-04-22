#ifndef JVR_APP_TIME_H
#define JVR_APP_TIME_H

#include "../common.h"
#include "../utils/timer.h"

JVR_NAMESPACE_OPEN_SCOPE

class Time {

public:
  enum PlaybackValue {
    PLAYBACK_WAITING,
    PLAYBACK_NEXT,
    PLAYBACK_PREVIOUS,
    PLAYBACK_LAST,
    PLAYBACK_FIRST,
    PLAYBACK_STOP
  };

  void Init(float start, float end, float fps);
  inline float GetMinTime(){return _minTime;};
  inline float GetStartTime(){return _startTime;};
  inline float GetEndTime(){return _endTime;};
  inline float GetMaxTime(){return _maxTime;};
  inline float GetActiveTime(){return _activeTime;};
  inline float GetFPS(){return _fps;};
  inline float GetSpeed(){return _speed;};
  inline bool  GetLoop(){return _loop;};
  inline float GetFramerate() { return _framerate; };
  inline float GetFrameDuration() { return _frame;};

  inline void SetMinTime(float time){_minTime = time;};
  inline void SetStartTime(float time){_startTime = time;};
  inline void SetEndTime(float time){_endTime = time;};
  inline void SetMaxTime(float time){_maxTime = time;};
  inline void SetActiveTime(float time){_activeTime = time;};
  inline void SetFPS(float fps){_fps = fps; _frame = 1.f/_fps;};
  inline void SetSpeed(float speed){_speed = speed;};
  inline void SetLoop(bool loop){_loop = loop;};

  void PreviousFrame();
  void NextFrame();
  void FirstFrame();
  void LastFrame();
  void StartPlayback(bool backward=false);
  void StopPlayback();
  int Playback();
  bool IsPlaying(){return _playback;};
  
  void ComputeFramerate(double T);

  // singleton 
  static Time *Get();

private:
  uint64_t                          _t;
  double                            _chronometer;
  float                             _activeTime;
  float                             _startTime;
  float                             _endTime;
  float                             _minTime;
  float                             _maxTime;
  float                             _fps;
  float                             _speed;
  float                             _frame;
  bool                              _loop;
  bool                              _playForwardOrBackward;
  bool                              _playback;

  double                            _lastT;
  size_t                            _frameCount;
  size_t                            _framerate;

  static Time*                      _singleton;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APP_SELECTION_H