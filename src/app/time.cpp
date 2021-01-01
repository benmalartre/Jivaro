#include "time.h"

AMN_NAMESPACE_OPEN_SCOPE

void Time::Init(float start, float end, float fps)
{
  _startTime = start;
  _minTime = _startTime;
  _activeTime = _startTime;
  _endTime = end;
  _maxTime = _endTime;
  _loop = false;
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
  std::cout << "SPEED = " << _speed;
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

void Time::StartPlayBack(bool backward)
{
  _stopWatch.Reset();
  _playback = true;
  _stopWatch.Start();
  _playForwardOrBackward = backward;
  PlayBack();
}

void Time::StopPlayBack()
{
  _stopWatch.Stop();
  _playback=false;
}

void Time::PlayBack()
{
  _stopWatch.Stop();
  std::cout << "PLAYBACK CURRENT fRAME : " << _activeTime << std::endl;
  if(_stopWatch.GetMilliseconds()>1000/_fps)
  {
    if(_playForwardOrBackward)PreviousFrame();
    else NextFrame();
    _stopWatch.Reset();
    _stopWatch.Start();
  }
} 
AMN_NAMESPACE_CLOSE_SCOPE

