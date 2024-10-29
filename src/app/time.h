#ifndef JVR_APP_TIME_H
#define JVR_APP_TIME_H

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Time {

public:
  enum PlaybackValue {
    PLAYBACK_WAITING,
    PLAYBACK_IDLE,
    PLAYBACK_NEXT,
    PLAYBACK_PREVIOUS,
    PLAYBACK_LAST,
    PLAYBACK_FIRST,
    PLAYBACK_STOP
  };

  enum PlaybackMode {
    PLAYBACK_REALTIME,
    PLAYBACK_ALLFRAMES
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
  inline float GetFrameDuration() { return static_cast<float>(_frame) * 1e-6;};
  inline int   GetMode(){return _mode;};

  inline void SetMinTime(float time){_minTime = time;};
  inline void SetStartTime(float time){_startTime = time;};
  inline void SetEndTime(float time){_endTime = time;};
  inline void SetMaxTime(float time){_maxTime = time;};
  inline void SetActiveTime(float time){_activeTime = time;};
  inline void SetMode(int mode){_mode = mode;};
  inline void SetSpeed(float speed){_speed = speed;};
  inline void SetLoop(bool loop){_loop = loop;};

  void SetFPS(float fps);

  bool PreviousFrame();
  bool NextFrame();
  void FirstFrame();
  void LastFrame();
  void StartPlayback(bool backward=false);
  void StopPlayback();
  int Playback();
  bool IsPlaying(){return _playback;};
  
  // singleton 
  static Time *Get();

protected:
  float _GetIncrement();

private:
  float                             _activeTime;          // current active time
  float                             _startTime;           // playback start time
  float                             _endTime;             // playback end time
  float                             _minTime;             // scene min time
  float                             _maxTime;             // scene max time
  float                             _fps;                 // frame per second
  float                             _speed;               // playback speed multiplier
  int                               _mode;                // playback mode : REALTIME or ALLFRAMES
  bool                              _loop;                // playback looping
  bool                              _backward;            // play backward
  bool                              _playback;            // currently playing

  uint64_t                          _frame;               // frame duration in micro seconds
  uint64_t                          _lastT;               // last clock time

  static Time*                      _singleton;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APP_SELECTION_H