#ifndef JVR_UI_TIMELINE_H
#define JVR_UI_TIMELINE_H

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/utils.h"
#include "../utils/icons.h"


PXR_NAMESPACE_OPEN_SCOPE
extern IconList ICONS;

#define SLIDER_THICKNESS 2.f

struct TimeData {
  float currentTime;
  float startTime;
  float endTime;
  float minTime;
  float maxTime;
  float fps;
  float speed;
  bool  loop;
  bool  playing;
};

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
  TimeData& GetData() { return _data; };
  const TimeData& GetData() const { return _data; };

private:
  float _GetFrameUnderMouse(int x, int y);
  pxr::GfVec2f _TimeToPosition(float time);
  float _PositionToTime(const pxr::GfVec2f& position);
  
  TimeData                _data;
  int                     _frame;
  int                     _lastFrame;
  double                  _lastX;
  double                  _lastY;
  static ImGuiWindowFlags _flags;
};


PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_TIMELINE_H