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


void Time::Init(float start, float end, float fps)
{
  _startTime = start;
  _minTime = _startTime;
  _activeTime = _startTime;
  _endTime = end;
  _maxTime = _endTime;
  _loop = true;
  _fps = fps > 1e-6 ? fps : 1e-6;
  _frame = 1.f / _fps;
  _speed = 1.f;
  _playback = false;
  _mode = PLAYBACK_REALTIME;
  _backward = false;
  _lastT = 0;
}

void Time::SetFPS(float fps)
{
  _fps = fps > 1e-6 ? fps : 1e-6; 
  _frame = 1.f/_fps;
  Geometry::SetFrameDuration(_frame);
};

// time
void Time::PreviousFrame()
{
  _lastT = CurrentTime();
  float currentTime = _activeTime - _speed * _frame;
  if(currentTime < _startTime)
  {
    if(_loop)_activeTime = _endTime;
    else _activeTime = _startTime;
  }
  else _activeTime = currentTime;
}

void Time::NextFrame()
{
  _lastT = CurrentTime();
  float currentTime = _activeTime + _speed * _frame;
  if(currentTime > _endTime)
  {
    if(_loop)_activeTime = _startTime;
    else _activeTime = _endTime;
  }
  else _activeTime = currentTime;
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
  if(_mode == PLAYBACK_REALTIME && (CurrentTime() - _lastT < _frame))
    return PLAYBACK_WAITING;

  if(!_playback)return PLAYBACK_IDLE;

  _lastT = CurrentTime();

  if(_backward) {
    if((_activeTime - _speed * _frame) < _startTime) {
      if(_loop) LastFrame();
      else StopPlayback();
      return _loop ? PLAYBACK_LAST : PLAYBACK_STOP;
    } else {
      PreviousFrame();
      return PLAYBACK_PREVIOUS;
    }
  } else {
    if((_activeTime + _speed * _frame) > _endTime) {
      if(_loop) FirstFrame();
      else StopPlayback();
      return _loop ? PLAYBACK_FIRST : PLAYBACK_STOP;
    } else {
      NextFrame();
      return PLAYBACK_NEXT;
    }
  } 
}

JVR_NAMESPACE_CLOSE_SCOPE
