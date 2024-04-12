#ifndef JVR_APP_TIME_H
#define JVR_APP_TIME_H

#pragma once

#include "../common.h"
#include "pxr/base/tf/stopwatch.h"

JVR_NAMESPACE_OPEN_SCOPE


class Time {
public:
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

  inline void SetMinTime(float time){_minTime = time;};
  inline void SetStartTime(float time){_startTime = time;};
  inline void SetEndTime(float time){_endTime = time;};
  inline void SetMaxTime(float time){_maxTime = time;};
  inline void SetActiveTime(float time){_activeTime = time;};
  inline void SetFPS(float fps){_fps = fps;};
  inline void SetSpeed(float speed){_speed = speed;};
  inline void SetLoop(bool loop){_loop = loop;};

  void PreviousFrame();
  void NextFrame();
  void FirstFrame();
  void LastFrame();
  void StartPlayBack(bool backward=false);
  void StopPlayBack();
  bool PlayBack();
  bool IsPlaying(){return _playback;};
  
  void ComputeFramerate(double T);
private:
  pxr::TfStopwatch                  _stopWatch;
  float                             _activeTime;
  float                             _startTime;
  float                             _endTime;
  float                             _minTime;
  float                             _maxTime;
  float                             _fps;
  float                             _speed;
  bool                              _loop;
  bool                              _playForwardOrBackward;
  bool                              _playback;

  double                            _lastT;
  size_t                            _frameCount;
  size_t                            _framerate;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APP_SELECTION_H