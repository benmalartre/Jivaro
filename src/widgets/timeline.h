#pragma once

#include "../default.h"
#include "../utils.h"

AMN_NAMESPACE_OPEN_SCOPE

class TimelineUI : BaseUI
{
  float _currentTime;
  float _startTime;
  float _endTime;
  float _minTime;
  float _maxTime;
  float _fps;
  float _speed;
  bool  _loop;

};
IMGUI_API void FillBackground();

AMN_NAMESPACE_CLOSE_SCOPE