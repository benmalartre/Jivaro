#include <functional>
#include "../utils/icons.h"
#include "../ui/utils.h"
#include "../ui/timeline.h"
#include "../app/application.h"
#include "../app/time.h"
#include "../app/view.h"
#include "../app/window.h"


JVR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags TimelineUI::_flags = 
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoScrollbar |
  ImGuiWindowFlags_NoMove;

// constructor
TimelineUI::TimelineUI(View* parent) 
  : BaseUI(parent, UIType::TIMELINE)
{
  _parent->SetFlag(View::TIMEVARYING);
  _parent->SetDirty();
}

// destructor
TimelineUI::~TimelineUI()
{
}

void 
TimelineUI::Init()
{
  Update();
  _parent->SetDirty();
  _initialized = true;
}
  
void 
TimelineUI::Update()
{
  Time* time = Time::Get();
  _minTime = time->GetMinTime();
  _startTime = time->GetStartTime();
  _endTime = time->GetEndTime();
  _maxTime = time->GetMaxTime();
  _currentTime = time->GetActiveTime();
  _loop = time->GetLoop();
  _fps = time->GetFPS();
  _speed = time->GetSpeed();
  _mode = time->GetMode();
  _playing = time->IsPlaying();
  _parent->SetDirty();
}

void 
TimelineUI::ValidateTime()
{
  Time* time = Time::Get();
  _currentTime = time->GetActiveTime();
  if (_minTime >= _maxTime)_maxTime = _minTime + 1;
  if (_endTime > _maxTime)_maxTime = _endTime;
  if (_startTime < _minTime)_startTime = _minTime;
  if (_endTime <= _startTime)_endTime = _startTime + 1;
  if (_currentTime < _startTime)_currentTime = _startTime;
  else if (_currentTime > _endTime)_currentTime = _endTime;
  
  //time->Lock();
  time->SetMinTime(_minTime);
  time->SetStartTime(_startTime);
  time->SetEndTime(_endTime);
  time->SetMaxTime(_maxTime);
  time->SetActiveTime(_currentTime);
  time->SetLoop(_loop);
  time->SetSpeed(_speed);
  time->SetMode(_mode);
  time->SetFPS(_fps);
  //time->Release();
}

float 
TimelineUI::_GetFrameUnderMouse(int x, int y)
{
  View* parent = GetView();
  return RESCALE(x - parent->GetX(), 0.f, parent->GetWidth(), _minTime, _maxTime);
}

GfVec2f 
TimelineUI::_TimeToPosition(float time)
{
  return GfVec2f(RESCALE(time,_minTime, _maxTime, 
    0.f, _parent->GetWidth()), 0.f);
}

float 
TimelineUI::_PositionToTime(const GfVec2f& position)
{
  return RESCALE(position[0] - _parent->GetX(), 0.f, 
    _parent->GetWidth(), _minTime, _maxTime);
}

void TimelineUI::MouseButton(int button, int action, int mods)
{
  Time* time = Time::Get();
  double x, y;
  glfwGetCursorPos(_parent->GetWindow()->GetGlfwWindow(), &x, &y);

  if (action == GLFW_PRESS) {
    if((_parent->GetMax()[1] - y) > TIMELINE_CONTROL_HEIGHT) {
      SetInteracting(true);
      _lastX = static_cast<double>(x);
      _lastY = static_cast<double>(y);
      _frame = _GetFrameUnderMouse(x, y);
    }
  } else if (action == GLFW_RELEASE) {
    if(IsInteracting()) {
      SetInteracting(false);
      _frame = _GetFrameUnderMouse(x, y);
      _currentTime = _frame;
      time->SetActiveTime(_frame);
    }
  }
  _parent->SetDirty();
}

void TimelineUI::MouseMove(int x, int y)
{
  _frame = _GetFrameUnderMouse(x, y);
  if (_interacting) {
    Time* time = Time::Get();
    _frame = _GetFrameUnderMouse(x, y);
    if (static_cast<int>(_frame) != static_cast<int>(_lastFrame)) {
      _currentTime = _frame;
      time->SetActiveTime(_frame);
      AttributeChangedNotice().Send();
      _lastFrame = _frame;
      _parent->SetDirty();
    }
  }
  if (_parent->GetFlag(View::INTERACTING) || _parent->GetFlag(View::OVER))
    _parent->SetDirty();
}

void TimelineUI::DrawButtons()
{
  Time* time = Time::Get();
  _playing = time->IsPlaying();
  UI::AddIconButton(0, ICON_FA_BACKWARD_FAST , UI::STATE_DEFAULT,
    [&](){
      _currentTime = _startTime;
      time->SetActiveTime(_currentTime);
      WindowRegistry::Get()->SetAllWindowsDirty();
    });
  ImGui::SameLine();

  UI::AddIconButton(1, ICON_FA_BACKWARD_STEP, UI::STATE_DEFAULT,
    [&](){
      time->PreviousFrame();
      WindowRegistry::Get()->SetAllWindowsDirty();
    });
  ImGui::SameLine();

  if (!_playing) {
    UI::AddCheckableIconButton(2, ICON_FA_PLAY , UI::STATE_DEFAULT,
    [&](){
      _playing = 1 - _playing;
      time->StartPlayback();
    });
  } else {
    UI::AddCheckableIconButton(2, ICON_FA_STOP , UI::STATE_SELECTED,
    [&](){
      _playing = 1 - _playing;
      time->StopPlayback();
    });
  }
  ImGui::SameLine();

  UI::AddIconButton(3, ICON_FA_FORWARD_STEP, UI::STATE_DEFAULT,
    [&](){
      time->NextFrame();
      WindowRegistry::Get()->SetAllWindowsDirty();
    });
  ImGui::SameLine();

  UI::AddIconButton(4, ICON_FA_FORWARD_FAST, UI::STATE_DEFAULT,
    [&](){
      _currentTime = _endTime;
      time->SetActiveTime(_currentTime);
      WindowRegistry::Get()->SetAllWindowsDirty();
    });
  ImGui::SameLine();

  UI::AddCheckableIconButton(5, ICON_FA_ROTATE,
    _loop ? UI::STATE_SELECTED : UI::STATE_DEFAULT,
    [&](){
      _loop = 1 - _loop;
      time->SetLoop(_loop);
    });
  ImGui::SameLine();
}


// this should move to UIUtils
void TimelineUI::_DrawOneControl(const char* name, float width, float& value, 
  float previous, short labelled, const char* tooltip)
{
  if(labelled == 1) {
    ImGui::Text(name);
    ImGui::SameLine();
  }
  ImGui::SetNextItemWidth(width);
  ImGui::InputScalar(UI::HiddenLabel(name).c_str(), ImGuiDataType_Float, &value,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if (!ImGui::IsItemActive() && value != previous)
    ValidateTime();
  
  if (tooltip && ImGui::IsItemHovered())
    AttachTooltip(tooltip);
  ImGui::SameLine();

  if(labelled == 2) {
    ImGui::Text(name);
    ImGui::SameLine();
  }
}

void TimelineUI::DrawControls()
{
  Time* time = Time::Get();
  int width = GetWidth();
  int height = GetHeight();

  ImGuiStyle* style = &ImGui::GetStyle();
  const ImVec4* colors = style->Colors;

  static const int padding = 4;

  int itemWidth = 40;
  float perc = width / 100.f;
  int posY = height - (TIMELINE_CONTROL_HEIGHT - padding);
  ImGui::SetCursorPosX(padding);
  ImGui::SetCursorPosY(posY);

  _DrawOneControl("min", itemWidth, _minTime, time->GetMinTime(), 2);
  _DrawOneControl("start", itemWidth, _startTime, time->GetStartTime(), 2);

  ImGui::SetCursorPosX(20 * perc);
  _DrawOneControl("fps", itemWidth, _fps, time->GetFPS(), 1);
  _DrawOneControl("speed", itemWidth, _speed, time->GetSpeed(), 1);

  // buttons
  ImGui::SetCursorPosX(40 * perc);
  DrawButtons();
  _DrawOneControl("time", itemWidth, _currentTime, time->GetActiveTime(), 0);

  ImGui::SetCursorPosX(60 * perc);
  static const char* playbackModeNames[2] = {"Realtime", "AllFrames"};
  UI::AddComboWidget("mode", &playbackModeNames[0], 2, _mode, 120);
  ImGui::SameLine();

  int textWidth = ImGui::CalcTextSize("endmax")[0];
  ImGui::SetCursorPosX(width - (2 * itemWidth + padding + textWidth));
  _DrawOneControl("end", itemWidth, _endTime, time->GetEndTime(), 1);
  _DrawOneControl("max", itemWidth, _maxTime, time->GetMaxTime(), 1);

}

void TimelineUI::DrawTimeSlider()
{
  ImDrawList* drawList = ImGui::GetWindowDrawList();

  float xmin = _parent->GetMin()[0];
  float xmax = _parent->GetMax()[0];
  float ymin = _parent->GetMin()[1];
  float ymax = _parent->GetMax()[1] - TIMELINE_CONTROL_HEIGHT;
  float ymid = ymax;//ymin * 0.25 + ymax * 0.75;

  float rounding = 8.f;
  float th = 1.f;
  const ImDrawCornerFlags corners_none = 0;
  const ImDrawCornerFlags corners_all = ImDrawCornerFlags_All;

  ImGuiStyle* style = &ImGui::GetStyle();
  const ImVec4* colors = style->Colors;
  static ImColor backColor(colors[ImGuiCol_FrameBgHovered]);
  static ImColor frontColor(colors[ImGuiCol_FrameBgActive]);
  static ImColor frameColor(colors[ImGuiCol_Text]);
  static ImColor sliderColor(colors[ImGuiCol_PlotHistogram]);

  // draw background
  drawList->AddRectFilled(
    GfVec2f(xmin, ymin),
    GfVec2f(xmax, ymax),
    backColor, rounding, corners_none
  );

  // draw frames
  drawList->AddRectFilled(
    GfVec2f(xmin, ymid),
    GfVec2f(xmax, ymax),
    frontColor, rounding, corners_none);

  Time* time = Time::Get();

  xmin += 2 * TIMELINE_SLIDER_THICKNESS;
  xmax -= 2 * TIMELINE_SLIDER_THICKNESS;

  static const float minBlockWidth = 16.f;
  int numFrames = (time->GetEndTime() - time->GetStartTime());
  const float currentBlockWidth = (float)GetWidth() / (float)numFrames;

  int numFramesToSkip = GfMax(1.f, GfFloor(minBlockWidth/currentBlockWidth));

  float incr = 1.f / (float)numFrames; 
  for (int i = 0; i < numFrames; ++i)
  {
    if (i % numFramesToSkip > 0)continue;
    float perc = i * incr;
    if (((int)(i - time->GetStartTime()) % (int)_fps) == 0)
    {
      GfVec2f p1(xmin * (1 - perc) + xmax * perc, ymin);
      GfVec2f p2(xmin * (1 - perc) + xmax * perc, ymid);
      drawList->AddLine(p1, p2, frameColor, 1);
    }
    else
    {
      GfVec2f p1(xmin * (1 - perc) + xmax * perc, ymin * 0.30 + ymid * 0.70);
      GfVec2f p2(xmin * (1 - perc) + xmax * perc, ymid);
      drawList->AddLine(p1, p2, frameColor, 1);
    }
  }

  // draw slider
  float sliderPerc =
    (float)(time->GetActiveTime() - time->GetStartTime()) /
    (float)(time->GetEndTime() - time->GetStartTime());
  float sliderX = (xmin * (1 - sliderPerc) + xmax * sliderPerc);
  drawList->AddRectFilled(
    GfVec2f(sliderX - TIMELINE_SLIDER_THICKNESS, ymin),
    GfVec2f(sliderX + TIMELINE_SLIDER_THICKNESS, ymid),
    sliderColor, rounding, corners_all
  );

  // draw frame
  /*
  const GfVec2f framePosition(_TimeToPosition(_frame));
  drawList->AddRectFilled(
    GfVec2f(framePosition[0]- 2.f, 0.f),
    GfVec2f(framePosition[0] + 2.f, ymid),
    ImColor(255, 0, 0, 255), rounding, corners_none);
    */
}

bool TimelineUI::Draw()
{
  if (!_initialized)Init();

  ImGui::SetNextWindowPos(_parent->GetMin());
  ImGui::SetNextWindowSize(_parent->GetSize());
  ImGui::Begin(_name.c_str(), NULL, _flags);
  

  DrawTimeSlider();
  DrawControls();

  ImGui::End();
  
  return 
    ImGui::IsAnyItemHovered() ||
    ImGui::IsAnyItemActive() ||
    ImGui::IsAnyItemFocused();
}

JVR_NAMESPACE_CLOSE_SCOPE