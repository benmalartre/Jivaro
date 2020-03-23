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

    // callbacks
    int StartFrameChanged(ImGuiTextEditCallbackData* data);

  private:
    float _currentTime;
    float _startTime;
    float _endTime;
    float _minTime;
    float _maxTime;
    float _fps;
    float _speed;
    bool  _loop;

};

AMN_NAMESPACE_CLOSE_SCOPE