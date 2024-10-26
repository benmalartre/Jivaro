#include "../utils/timer.h"
#include "../geometry/geometry.h"
#include "../app/time.h"


JVR_NAMESPACE_OPEN_SCOPE

// singleton
//----------------------------------------------------------------------------
Time* Time::_singleton=nullptr;

Time* Time::Get() { 
  if(_singleton==nullptr){
    _singleton = new Time();
  }
  return _singleton; 
};


float Time::_GetIncrement()
{
  return _speed * static_cast<float>(_frame) * 1e-6 * _fps;
};

void Time::Init(float start, float end, float fps)
{
  _startTime = start;
  _minTime = _startTime;
  _activeTime = _startTime;
  _endTime = end;
  _maxTime = _endTime;
  _loop = true;
  _fps = fps > 1 ? fps : 1;
  _frame = 1e6 / _fps;
  _speed = 1.f;
  _playback = false;
  _mode = PLAYBACK_REALTIME;
  _backward = false;
  _lastT = 0;
}

void Time::SetFPS(float fps)
{
  if(fps == _fps) return;
  _fps = fps > 1 ? fps : 1; 
  _frame = 1e6 / _fps;
  Geometry::SetFrameDuration(static_cast<float>(_frame) * 1e-6);
};

// time
bool Time::PreviousFrame()
{
  _lastT = CurrentTime();
  float currentTime = _activeTime - _GetIncrement();
  if(currentTime < _startTime) {
    _activeTime = _loop ? _endTime : _startTime;
    return _loop;
  }
  _activeTime = currentTime;
  return true;
}

bool Time::NextFrame()
{
  std::cout << "NEXT FRAME " << CurrentTime() << " " << _lastT << " " << _GetIncrement() << std::endl;
  _lastT = CurrentTime();
  float currentTime = _activeTime + _GetIncrement();
  if(currentTime > _endTime) {
    _activeTime = _loop ? _startTime : _endTime;
    return _loop;
  }
  _activeTime = currentTime;
  return true;
}

void Time::FirstFrame()
{
  _lastT = CurrentTime();
  _activeTime = _startTime;
}

void Time::LastFrame()
{
  _lastT = CurrentTime();
  _activeTime = _endTime;
}

void Time::StartPlayback(bool backward)
{
  _lastT = CurrentTime();
  _playback = true;
  _backward = backward;
  Playback();
}

void Time::StopPlayback()
{
  _playback=false;
}

int Time::Playback()
{
  // check for real time
  if(_mode == PLAYBACK_REALTIME && ((CurrentTime() - _lastT) < _frame))
    return PLAYBACK_WAITING;

  // idle
  if(!_playback)return PLAYBACK_IDLE;

  // playing
  if (_backward) return (PreviousFrame() ? PLAYBACK_PREVIOUS : PLAYBACK_STOP);
  else return (NextFrame() ? PLAYBACK_NEXT : PLAYBACK_STOP);
    
}

JVR_NAMESPACE_CLOSE_SCOPE
