#include "../app/time.h"

JVR_NAMESPACE_OPEN_SCOPE

void Time::Init(float start, float end, float fps)
{
  _startTime = start;
  _minTime = _startTime;
  _activeTime = _startTime;
  _endTime = end;
  _maxTime = _endTime;
  _loop = true;
  _fps = fps;
  _speed = 1.f;
  _playback = false;
  _playForwardOrBackward = false;
  _lastT = 0;
  _frameCount = 0;
}

void Time::ComputeFramerate(double T)
{
  _frameCount++;
  if (T - _lastT >= 1.0)
  {
    _framerate = _frameCount;
    _frameCount = 0;
    _lastT = T;
  }
}
// time
void Time::PreviousFrame()
{
  float currentTime = _activeTime - _speed;
  if(currentTime < _startTime)
  {
    if(_loop)_activeTime = _endTime;
    else _activeTime = _startTime;
  }
  else _activeTime = currentTime;
}

void Time::NextFrame()
{
  float currentTime = _activeTime + _speed;
  if(currentTime > _endTime)
  {
    if(_loop)_activeTime = _startTime;
    else _activeTime = _endTime;
  }
  else _activeTime = currentTime;
}

void Time::FirstFrame()
{
  _activeTime = _startTime;
}

void Time::LastFrame()
{
  _activeTime = _endTime;
}

void Time::StartPlayback(bool backward)
{
  _t = CurrentTime();
  _chronometer = 0.0;
  _playback = true;
  _playForwardOrBackward = backward;
  Playback();
}

void Time::StopPlayback()
{
  _playback=false;
}

int Time::Playback()
{
  uint64_t t = CurrentTime();
  _chronometer += (t - _t) * 1e-9;
  _t = t;

  if(_chronometer < (1.f/_fps))
    return PLAYBACK_WAITING;

  _chronometer = 0.f;
  
  if(!_playForwardOrBackward && _activeTime > _endTime)
    return _loop ? PLAYBACK_FIRST : PLAYBACK_STOP;

  else if(_playForwardOrBackward && _activeTime < _startTime)
    return _loop ? PLAYBACK_LAST : PLAYBACK_STOP;

  else if(!_playForwardOrBackward)
    return PLAYBACK_NEXT;

  else return PLAYBACK_PREVIOUS;

  
} 

JVR_NAMESPACE_CLOSE_SCOPE

