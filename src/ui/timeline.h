#pragma once

#include "../common.h"
#include "../app/ui.h"
#include "../utils/utils.h"
#include "../utils/icons.h"


AMN_NAMESPACE_OPEN_SCOPE
extern AmnIconMap AMN_ICONS;

class TimelineUI : BaseUI
{
public:
  TimelineUI(View* parent);
  ~TimelineUI();

  // overrides
  void MouseButton(int button, int action, int mods) override;
  void MouseMove(int x, int y) override;
  bool Draw() override;

  void Init();
  void DrawControls();
  void DrawTimeSlider();
  void ValidateTime();
  void Update();

  static void StartStopPlayback(TimelineUI* ui);
  static void SimpleCallback();
  static void DifficultCallback(TimelineUI* ui, int x);
  static void VeryDifficultCallback(TimelineUI* ui, int x, float y, const char* z);

private:
  float               _currentTime;
  float               _startTime;
  float               _endTime;
  float               _minTime;
  float               _maxTime;
  float               _fps;
  float               _speed;
  bool                _loop;
  bool                _playing;

  bool                _interacting;
  double              _lastX;
  double              _lastY;
};


AMN_NAMESPACE_CLOSE_SCOPE