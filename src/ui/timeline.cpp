#include "timeline.h"
#include "../app/application.h"
#include "../app/time.h"
#include "../utils/icons.h"
#include <functional>


AMN_NAMESPACE_OPEN_SCOPE

// constructor
TimelineUI::TimelineUI(View* parent) :BaseUI(parent, "Timeline")
{
  _flags = ImGuiWindowFlags_None
    | ImGuiWindowFlags_NoResize
    | ImGuiWindowFlags_NoTitleBar
    | ImGuiWindowFlags_NoScrollbar
    | ImGuiWindowFlags_NoMove;
  _parent->SetDirty();
  _parent->SetFlag(View::FORCEREDRAW);
}

// destructor
TimelineUI::~TimelineUI()
{
}

void TimelineUI::Init()
{
  Update();
  _parent->SetDirty();
  _initialized = true;
}
  
void TimelineUI::Update()
{
  Application* app = AMN_APPLICATION;
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

void TimelineUI::ValidateTime()
{
  Application* app = AMN_APPLICATION;
  Time& time = app->GetTime();

  _currentTime = time.GetActiveTime();
  if (_minTime >= _maxTime)_maxTime = _minTime + 1;
  if (_endTime>_maxTime)_endTime = _maxTime;
  if (_startTime < _minTime)_startTime = _minTime;
  if (_endTime <= _startTime)_endTime = _startTime + 1;
  if (_maxTime <= _endTime)_maxTime = _endTime;
  if (_currentTime < _startTime)_currentTime = _startTime;
  else if (_currentTime > _endTime)_currentTime = _endTime;

  
  time.SetMinTime(_minTime);
  time.SetStartTime(_startTime);
  time.SetEndTime(_endTime);
  time.SetMaxTime(_maxTime);
  time.SetActiveTime(_currentTime);
  time.SetLoop(_loop);
}

void TimelineUI::PlaybackCallback(TimelineUI* ui)
{
  ui->_playing = 1 - ui->_playing;
}

void TimelineUI::FirstFrameCallback(TimelineUI* ui)
{
  std::cout << "FIRST FRAME CALLBACK !!" << std::endl;
}

void TimelineUI::LastFrameCallback(TimelineUI* ui)
{
  std::cout << "LAST FRAME CALLBACK !!" << std::endl;
}

void TimelineUI::LoopCallback(TimelineUI* ui)
{
  ui->_loop = 1 - ui->_loop;
  Application* app = AMN_APPLICATION;
  Time& time = app->GetTime();
  time.SetLoop(ui->_loop);
}

void TimelineUI::MouseButton(int button, int action, int mods)
{
  if (action == GLFW_PRESS)
    _parent->SetFlag(View::INTERACTING);
  else if (action == GLFW_RELEASE)
    _parent->ClearFlag(View::INTERACTING);

  _parent->SetDirty();
}

void TimelineUI::MouseMove(int x, int y)
{
  if (_parent->GetFlag(View::INTERACTING) || _parent->GetFlag(View::OVER))
    _parent->SetDirty();
}

void TimelineUI::DrawControls()
{
  Application* app = AMN_APPLICATION;
  int width = GetWidth();
  int height = GetHeight();

  ImGuiStyle* style = &ImGui::GetStyle();
  const ImVec4* colors = style->Colors;

  ImGui::SetCursorPosX(20);
  ImGui::SetCursorPosY(height - 20);

  ImGui::PushFont(GetWindow()->GetMediumFont(0));

  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##minTime", ImGuiDataType_Float, &_minTime,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if (!ImGui::IsItemActive() && _minTime != app->GetTime().GetMinTime())
  {
    ValidateTime();
  }
  AttachTooltip("Minimum Time", 0.5f, 128, GetWindow()->GetRegularFont(0));
  ImGui::SameLine(); 

  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##startTime", ImGuiDataType_Float, &_startTime,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if (!ImGui::IsItemActive() && _startTime != app->GetTime().GetStartTime())
  {
    ValidateTime();
  }
  AttachTooltip("Start Time", 0.5f, 128, GetWindow()->GetRegularFont(0));
  ImGui::SameLine(); 

  float cy = ImGui::GetCursorPosY();
  // play button
  Icon* icon = NULL;
  icon = &AMN_ICONS[AMN_ICON_SMALL]["firstframe.png"];
  AddIconButton<IconPressedFunc>(
    icon,
    (IconPressedFunc)FirstFrameCallback, this
    );

  if (!_playing) icon = &AMN_ICONS[AMN_ICON_SMALL]["playforward.png"];
  else icon = &AMN_ICONS[AMN_ICON_SMALL]["stop.png"];

  AddIconButton<IconPressedFunc, TimelineUI*>(
    icon,
    (IconPressedFunc)PlaybackCallback, this);

  icon = &AMN_ICONS[AMN_ICON_SMALL]["lastframe.png"];
  AddIconButton<IconPressedFunc, TimelineUI*>(
    icon,
    (IconPressedFunc)LastFrameCallback, this);

  icon = &AMN_ICONS[AMN_ICON_SMALL]["loop.png"];
  AddCheckableIconButton<IconPressedFunc, TimelineUI*>(
    icon,
    _loop,
    (IconPressedFunc)LoopCallback, this);

  ImGui::SetCursorPosY(cy);

  // current time
  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##currentTime", ImGuiDataType_Float, &_currentTime,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if (!ImGui::IsItemActive() && _currentTime != app->GetTime().GetActiveTime())
  {
    ValidateTime();
  }
  AttachTooltip("Current Time", 0.5f, 128, GetWindow()->GetRegularFont(0));
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
  AttachTooltip("Maximum Time", 0.5f, 128, GetWindow()->GetRegularFont(0));
  ImGui::SameLine();
  ImGui::PopFont();
}

void TimelineUI::DrawTimeSlider()
{
  ImDrawList* drawList = ImGui::GetWindowDrawList();

  const float xmin = _parent->GetMin()[0];
  const float xmax = _parent->GetMax()[0];
  const float ymin = _parent->GetMin()[1];
  const float ymax = _parent->GetMax()[1] - 30;
  const float ymid = ymin * 0.25 + ymax * 0.75;

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

  Application* app = AMN_APPLICATION;

  int numFrames = (app->GetTime().GetEndTime() - app->GetTime().GetStartTime());
  float incr = 1 / (float)numFrames;
  for (int i = 0; i < numFrames; ++i)
  {
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
    pxr::GfVec2f(sliderX - 2, ymin),
    pxr::GfVec2f(sliderX + 2, ymid),
    sliderColor, rounding, corners_all
  );
}

bool TimelineUI::Draw()
{
  if (!_initialized)Init();
  ImGui::Begin(_name.c_str(), NULL, _flags);

  ImGui::SetWindowPos(_parent->GetMin());
  ImGui::SetWindowSize(_parent->GetSize());

  DrawTimeSlider();
  DrawControls();

  ImGui::End();
  
  return 
    ImGui::IsAnyItemHovered() ||
    ImGui::IsAnyItemActive() ||
    ImGui::IsAnyItemFocused();
}

AMN_NAMESPACE_CLOSE_SCOPE