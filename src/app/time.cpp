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
  _frame = 1000.f / _fps;
  _speed = 1.f;
  _playback = false;
  _playForwardOrBackward = false;
  _lastT = 0;
}

void Time::SetFPS(float fps)
{
  _fps = fps > 1e-6 ? fps : 1e-6; 
  _frame = 1000.f/_fps;
  Geometry::SetFrameDuration(_frame);
};

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
  _chronometer = 0;
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
  _chronometer = (double)(CurrentTime() - _lastT) * 1e-6f;

  if(_chronometer < _frame)
    return PLAYBACK_WAITING;

  _lastT = CurrentTime();
  
  if(!_playForwardOrBackward && _activeTime > _endTime)
    return _loop ? PLAYBACK_FIRST : PLAYBACK_STOP;

  else if(_playForwardOrBackward && _activeTime < _startTime)
    return _loop ? PLAYBACK_LAST : PLAYBACK_STOP;

  else if(!_playForwardOrBackward)
    return PLAYBACK_NEXT;

  else return PLAYBACK_PREVIOUS;

  
} 

JVR_NAMESPACE_CLOSE_SCOPE

