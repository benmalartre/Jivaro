#ifndef JVR_UI_TIMELINE_H
#define JVR_UI_TIMELINE_H

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/utils.h"
#include "../utils/icons.h"


JVR_NAMESPACE_OPEN_SCOPE
extern IconList ICONS;

#define SLIDER_THICKNESS 2.f

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
  void DrawButtons();
  void DrawTimeSlider();
  void ValidateTime();
  void Update();

  float                   _currentTime;
  float                   _startTime;
  float                   _endTime;
  float                   _minTime;
  float                   _maxTime;
  float                   _fps;
  float                   _speed;
  bool                    _loop;
  bool                    _playing;
  bool                    _interacting;
  double                  _lastX;
  double                  _lastY;
  static ImGuiWindowFlags _flags;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_TIMELINE_H