#pragma once

#include "../default.h"
#include "../app/ui.h"

AMN_NAMESPACE_OPEN_SCOPE

class TimelineUI : BaseUI
{
  public:
    TimelineUI(View* parent);
    ~TimelineUI();
    
    // overrides
    void MouseButton(int action, int button, int mods) override;
    void MouseMove(int x, int y) override;
    void Draw() override;

    void DrawControls();
    void DrawTimeSlider();
    void ValidateMinMaxTime();

  private:
    float _currentTime;
    float _currentTimeEdit;
    float _startTime;
    float _startTimeEdit;
    float _endTime;
    float _endTimeEdit;
    float _minTime;
    float _minTimeEdit;
    float _maxTime;
    float _maxTimeEdit;
    float _fps;
    float _speed;
    bool  _loop;

};

AMN_NAMESPACE_CLOSE_SCOPE