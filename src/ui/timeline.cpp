#include <functional>
#include "../utils/icons.h"
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
  Application* app = GetApplication();
  Time& time = app->GetTime();
  _minTime = time.GetMinTime();
  _startTime = time.GetStartTime();
  _endTime = time.GetEndTime();
  _maxTime = time.GetMaxTime();
  _currentTime = time.GetActiveTime();
  _loop = time.GetLoop();
  _fps = time.GetFPS();
  _playing = time.IsPlaying();
  _parent->SetDirty();
}

void 
TimelineUI::ValidateTime()
{
  Time& time = GetApplication()->GetTime();

  _currentTime = time.GetActiveTime();
  if (_minTime >= _maxTime)_maxTime = _minTime + 1;
  if (_endTime > _maxTime)_maxTime = _endTime;
  if (_startTime < _minTime)_startTime = _minTime;
  if (_endTime <= _startTime)_endTime = _startTime + 1;
  if (_currentTime < _startTime)_currentTime = _startTime;
  else if (_currentTime > _endTime)_currentTime = _endTime;
  
  time.SetMinTime(_minTime);
  time.SetStartTime(_startTime);
  time.SetEndTime(_endTime);
  time.SetMaxTime(_maxTime);
  time.SetActiveTime(_currentTime);
  time.SetLoop(_loop);
}

float 
TimelineUI::_GetFrameUnderMouse(int x, int y)
{
  View* parent = GetView();
  return RESCALE(x - parent->GetX(), 0.f, parent->GetWidth(), _minTime, _maxTime);
}

pxr::GfVec2f 
TimelineUI::_TimeToPosition(float time)
{
  return pxr::GfVec2f(RESCALE(time,_minTime, _maxTime, 
    0.f, _parent->GetWidth()), 0.f);
}

float 
TimelineUI::_PositionToTime(const pxr::GfVec2f& position)
{
  return RESCALE(position[0] - _parent->GetX(), 0.f, 
    _parent->GetWidth(), _minTime, _maxTime);
}

void TimelineUI::MouseButton(int button, int action, int mods)
{
  Time& time = GetApplication()->GetTime();
  double x, y;
  glfwGetCursorPos(_parent->GetWindow()->GetGlfwWindow(), &x, &y);

  if (action == GLFW_PRESS) {
    SetInteracting(true);
    _lastX = static_cast<double>(x);
    _lastY = static_cast<double>(y);
    _frame = _GetFrameUnderMouse(x, y);
  } else if (action == GLFW_RELEASE) {
    SetInteracting(false);
    _frame = _GetFrameUnderMouse(x, y);
    _currentTime = _frame;
    time.SetActiveTime(_frame);
  }
  _parent->SetDirty();
}

void TimelineUI::MouseMove(int x, int y)
{
  _frame = _GetFrameUnderMouse(x, y);
  if (_interacting) {
    Time& time = GetApplication()->GetTime();
    _frame = _GetFrameUnderMouse(x, y);
    if (static_cast<int>(_frame) != static_cast<int>(_lastFrame)) {
      _currentTime = _frame;
      time.SetActiveTime(_frame);
      AttributeChangedNotice().Send();
      _lastFrame = _frame;
      _parent->GetWindow()->ForceRedraw();
    }
  }
  if (_parent->GetFlag(View::INTERACTING) || _parent->GetFlag(View::OVER))
    _parent->SetDirty();
}

void TimelineUI::DrawButtons()
{
  Time& time = GetApplication()->GetTime();
  UIUtils::AddIconButton(0, ICON_FA_BACKWARD_FAST , ICON_DEFAULT,
    [&](){
      _currentTime = _startTime;
      time.SetActiveTime(_currentTime);
      GetApplication()->DirtyAllEngines();
    });
  ImGui::SameLine();

  UIUtils::AddIconButton(1, ICON_FA_BACKWARD_STEP, ICON_DEFAULT,
    [&](){
      time.PreviousFrame();
      GetApplication()->DirtyAllEngines();
    });
  ImGui::SameLine();

  if (!_playing) {
    UIUtils::AddCheckableIconButton(2, ICON_FA_PLAY , ICON_DEFAULT,
    [&](){
      _playing = 1 - _playing;
      time.StartPlayBack();
    });
  } else {
    UIUtils::AddCheckableIconButton(2, ICON_FA_STOP , ICON_SELECTED,
    [&](){
      _playing = 1 - _playing;
      time.StopPlayBack();
    });
  }
  ImGui::SameLine();

  UIUtils::AddIconButton(3, ICON_FA_FORWARD_STEP, ICON_DEFAULT,
    [&](){
      time.NextFrame();
      GetApplication()->DirtyAllEngines();
    });
  ImGui::SameLine();

  UIUtils::AddIconButton(4, ICON_FA_FORWARD_FAST, ICON_DEFAULT,
    [&](){
      _currentTime = _endTime;
      time.SetActiveTime(_currentTime);
      GetApplication()->DirtyAllEngines();
    });
  ImGui::SameLine();

  UIUtils::AddCheckableIconButton(5, ICON_FA_ROTATE,
    _loop ? ICON_SELECTED : ICON_DEFAULT,
    [&](){
      _loop = 1 - _loop;
      time.SetLoop(_loop);
    });
  ImGui::SameLine();
}

void TimelineUI::DrawControls()
{
  Application* app = GetApplication();
  int width = GetWidth();
  int height = GetHeight();

  ImGuiStyle* style = &ImGui::GetStyle();
  const ImVec4* colors = style->Colors;

  ImGui::SetCursorPosX(20);
  ImGui::SetCursorPosY(height - TIMELINE_CONTROL_HEIGHT + 8);

  //ImGui::PushFont(GetWindow()->GetMediumFont(0));

  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##minTime", ImGuiDataType_Float, &_minTime,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if (!ImGui::IsItemActive() && _minTime != app->GetTime().GetMinTime())
  {
    ValidateTime();
  }
  if (ImGui::IsItemHovered())
    AttachTooltip("Minimum Time");
  //AttachTooltip("Minimum Time", 0.5f, 128, GetWindow()->GetRegularFont(0));
  ImGui::SameLine(); 

  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##startTime", ImGuiDataType_Float, &_startTime,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if (!ImGui::IsItemActive() && _startTime != app->GetTime().GetStartTime())
  {
    ValidateTime();
  }
  if (ImGui::IsItemHovered())
    AttachTooltip("Start Time");
  //AttachTooltip("Start Time", 0.5f, 128, GetWindow()->GetRegularFont(0));
  ImGui::SameLine(); 

  float cy = ImGui::GetCursorPosY();
  //ImGui::SetCursorPosY(cy - 6);
  ImGui::SetCursorPosX(width * 0.5f - 64);

  // buttons
  DrawButtons();
  ImGui::SetCursorPosY(cy);

  // current time
  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##currentTime", ImGuiDataType_Float, &_currentTime,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if (!ImGui::IsItemActive() && _currentTime != app->GetTime().GetActiveTime())
  {
    ValidateTime();
  }
  if (ImGui::IsItemHovered())
    AttachTooltip("Current Time");
  //AttachTooltip("Current Time", 0.5f, 128, GetWindow()->GetRegularFont(0));
  ImGui::SameLine(); 

  ImGui::SetCursorPosX(width - 140);

  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##endTime", ImGuiDataType_Float, &_endTime,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if (!ImGui::IsItemActive() && _endTime != app->GetTime().GetEndTime())
  {
    ValidateTime();
  }
  ImGui::SameLine();

  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##maxTime", ImGuiDataType_Float, &_maxTime,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if (!ImGui::IsItemActive() && _maxTime != app->GetTime().GetMaxTime())
  {
    ValidateTime();
  }
  if (ImGui::IsItemHovered())
    AttachTooltip("Maximum Time");
  //AttachTooltip("Maximum Time", 0.5f, 128, GetWindow()->GetRegularFont(0));
  ImGui::SameLine();
  //ImGui::PopFont();
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
    pxr::GfVec2f(xmin, ymin),
    pxr::GfVec2f(xmax, ymax),
    backColor, rounding, corners_none
  );

  // draw frames
  drawList->AddRectFilled(
    pxr::GfVec2f(xmin, ymid),
    pxr::GfVec2f(xmax, ymax),
    frontColor, rounding, corners_none);

  Application* app = GetApplication();

  xmin += 2 * TIMELINE_SLIDER_THICKNESS;
  xmax -= 2 * TIMELINE_SLIDER_THICKNESS;

  static const float minBlockWidth = 16.f;
  int numFrames = (app->GetTime().GetEndTime() - app->GetTime().GetStartTime());
  const float currentBlockWidth = (float)GetWidth() / (float)numFrames;

  int numFramesToSkip = pxr::GfMax(1.f, pxr::GfFloor(minBlockWidth/currentBlockWidth));

  float incr = 1.f / (float)numFrames; 
  for (int i = 0; i < numFrames; ++i)
  {
    if (i % numFramesToSkip > 0)continue;
    float perc = i * incr;
    if (((int)(i - app->GetTime().GetStartTime()) % (int)_fps) == 0)
    {
      pxr::GfVec2f p1(xmin * (1 - perc) + xmax * perc, ymin);
      pxr::GfVec2f p2(xmin * (1 - perc) + xmax * perc, ymid);
      drawList->AddLine(p1, p2, frameColor, 1);
    }
    else
    {
      pxr::GfVec2f p1(xmin * (1 - perc) + xmax * perc, ymin * 0.30 + ymid * 0.70);
      pxr::GfVec2f p2(xmin * (1 - perc) + xmax * perc, ymid);
      drawList->AddLine(p1, p2, frameColor, 1);
    }
  }

  // draw slider
  float sliderPerc =
    (float)(app->GetTime().GetActiveTime() - app->GetTime().GetStartTime()) /
    (float)(app->GetTime().GetEndTime() - app->GetTime().GetStartTime());
  float sliderX = (xmin * (1 - sliderPerc) + xmax * sliderPerc);
  drawList->AddRectFilled(
    pxr::GfVec2f(sliderX - TIMELINE_SLIDER_THICKNESS, ymin),
    pxr::GfVec2f(sliderX + TIMELINE_SLIDER_THICKNESS, ymid),
    sliderColor, rounding, corners_all
  );

  // draw frame
  /*
  const pxr::GfVec2f framePosition(_TimeToPosition(_frame));
  drawList->AddRectFilled(
    pxr::GfVec2f(framePosition[0]- 2.f, 0.f),
    pxr::GfVec2f(framePosition[0] + 2.f, ymid),
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