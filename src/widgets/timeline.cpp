#include "timeline.h"
#include "../app/application.h"
#include "../utils/icons.h"
#include <functional>


AMN_NAMESPACE_OPEN_SCOPE

// constructor
TimelineUI::TimelineUI(View* parent) :BaseUI(parent, "Timeline")
{
  _flags = ImGuiWindowFlags_None
    | ImGuiWindowFlags_NoResize
    | ImGuiWindowFlags_NoTitleBar
    | ImGuiWindowFlags_NoMove;

  _parent->SetDirty();
  InitializeIcons();
}

// destructor
TimelineUI::~TimelineUI()
{
}

void TimelineUI::Init(Application* app)
{
  _app = app;
  Update();
  _parent->SetDirty();
}

void TimelineUI::Update()
{
  _minTime = _app->GetMinTime();
  _startTime = _app->GetStartTime();
  _endTime = _app->GetEndTime();
  _maxTime = _app->GetMaxTime();
  _currentTime = _app->GetCurrentTime();
  _loop = _app->GetLoop();
  _fps = _app->GetFPS();
  _playing = _app->IsPlaying();
  _parent->SetDirty();
}

void TimelineUI::ValidateTime()
{
  if (_minTime >= _maxTime)_maxTime = _minTime + 1;
  if (_endTime>_maxTime)_endTime = _maxTime;
  if (_startTime < _minTime)_startTime = _minTime;
  if (_endTime <= _startTime)_endTime = _startTime + 1;
  if (_maxTime <= _endTime)_maxTime = _endTime;
  if (_currentTime < _startTime)_currentTime = _startTime;
  else if (_currentTime > _endTime)_currentTime = _endTime;

  if (_app)
  {
    _app->SetMinTime(_minTime);
    _app->SetStartTime(_startTime);
    _app->SetEndTime(_endTime);
    _app->SetMaxTime(_maxTime);
    _app->SetCurrentTime(_currentTime);
    _app->SetLoop(_loop);
  }
}

void TimelineUI::StartStopPlayback(TimelineUI* ui)
{
  if (ui->_playing)
    std::cout << "MY FUCKIN STOP PLAY BACK!!!" << std::endl;
  else
    std::cout << "MY FUCKIN START PLAY BACK!!!" << std::endl;
  ui->_playing = 1 - ui->_playing;
}

void TimelineUI::SimpleCallback()
{
  std::cout << "DUMB DUMB :D!!!" << std::endl;
}

void TimelineUI::DifficultCallback(TimelineUI* ui, int x)
{
  std::cout << "YUUUUUUPPPPIIIIII :D!!!" << x << std::endl;
}

void TimelineUI::VeryDifficultCallback(TimelineUI* ui, int x, float y, const char* z)
{
  std::cout << "YUUUUUUPPPPIIIIII :D!!!" << x << "," << y << "," << z << std::endl;
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
  if (_parent->GetFlag(View::INTERACTING))
    _parent->SetDirty();
}

void TimelineUI::DrawControls(bool* changed)
{
  int width = GetWidth();
  int height = GetHeight();

  ImGuiStyle* style = &ImGui::GetStyle();
  const ImVec4* colors = style->Colors;

  ImGui::SetCursorPosX(20);
  ImGui::SetCursorPosY(height - 20);

  ImGui::PushFont(GetWindow()->GetMediumFont(0));

  ImGui::SetNextItemWidth(60);
  if (ImGui::InputScalar("##minTime", ImGuiDataType_Float, &_minTime,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll))*changed = true;
  if (!ImGui::IsItemActive() && _minTime != _app->GetMinTime())
  {
    ValidateTime();
  }
  ImGui::SameLine(); //HelpMarker("Minimum Time");

  ImGui::SetNextItemWidth(60);
  if (ImGui::InputScalar("##startTime", ImGuiDataType_Float, &_startTime,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll))*changed = true;
  if (!ImGui::IsItemActive() && _startTime != _app->GetStartTime())
  {
    ValidateTime();
  }
  ImGui::SameLine(); //HelpMarker("Start Time");

  // play button
  Icon* icon = NULL;
  if (!_playing) icon = &AMN_ICONS["play_btn.png"];
  else icon = &AMN_ICONS["stop_btn.png"];

  if(AddIconButton<IconPressedFunc, TimelineUI*>(
    icon,
    (IconPressedFunc)StartStopPlayback, this
    ))*changed=true;

  icon = &AMN_ICONS["first_frame_btn.png"];
  if(AddIconButton<IconPressedFunc>(
    icon,
    (IconPressedFunc)SimpleCallback
    ))*changed=true;

  icon = &AMN_ICONS["last_frame_btn.png"];
  if(AddIconButton<IconPressedFunc, TimelineUI*, int>(
    icon,
    (IconPressedFunc)DifficultCallback,
    this,
    666
    ))*changed=true;

  icon = &AMN_ICONS["loop_btn.png"];
  if(AddIconButton<IconPressedFunc, TimelineUI*, int, float, const char*>(
    icon,
    (IconPressedFunc)VeryDifficultCallback,
    this,
    666,
    7.654321,
    "SATAN"
    ))*changed=true;


  // current time
  ImGui::SetNextItemWidth(60);
  if (ImGui::InputScalar("##currentTime", ImGuiDataType_Float, &_currentTime,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll))*changed = true;
  if (!ImGui::IsItemActive() && _currentTime != _app->GetCurrentTime())
  {
    ValidateTime();
  }
  ImGui::SameLine(); //HelpMarker("Current Time");

                     // loop
  if (ImGui::Checkbox("Loop", &_loop)) {
    std::cout << "LOOP PRESSED !" << std::endl;
    *changed = true;
  }
  if (!ImGui::IsItemActive() && _loop != _app->GetLoop())
  {
    ValidateTime();
  }
  ImGui::SameLine(); //HelpMarker("Loop Playback");

  ImGui::SetCursorPosX(width - 140);

  ImGui::SetNextItemWidth(60);
  if (ImGui::InputScalar("##endTime", ImGuiDataType_Float, &_endTime,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll))*changed = true;
  if (!ImGui::IsItemActive() && _endTime != _app->GetEndTime())
  {
    ValidateTime();
  }
  ImGui::SameLine();

  ImGui::SetNextItemWidth(60);
  if (ImGui::InputScalar("##maxTime", ImGuiDataType_Float, &_maxTime,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll))*changed = true;
  if (!ImGui::IsItemActive() && _maxTime != _app->GetMaxTime())
  {
    ValidateTime();
  }
  ImGui::SameLine();
  ImGui::PopFont();
}

void TimelineUI::DrawTimeSlider()
{
  ImGui::SetCursorScreenPos(_parent->GetMin());

  ImDrawList* drawList = ImGui::GetWindowDrawList();

  const pxr::GfVec2f p = ImGui::GetCursorScreenPos();
  const float xmin = _parent->GetMin()[0];
  const float xmax = _parent->GetMax()[0];
  const float ymin = _parent->GetMin()[1];
  const float ymax = _parent->GetMax()[1] - 30;
  const float ymid = ymin * 0.25 + ymax * 0.75;

  const ImU32 col = rand();//ImColor(colf.x, colf.y, colf.z, 1.f);
  float rounding = 8.f;
  float th = 1.f;
  const ImDrawCornerFlags corners_none = 0;
  const ImDrawCornerFlags corners_all = ImDrawCornerFlags_All;

  ImGuiStyle* style = &ImGui::GetStyle();
  const ImVec4* colors = style->Colors;
  static ImColor backColor(colors[ImGuiCol_FrameBgHovered]);
  static ImColor frontColor(colors[ImGuiCol_FrameBgActive]);
  static ImColor frameColor(colors[ImGuiCol_PlotLines]);
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

  int numFrames = (_app->GetEndTime() - _app->GetStartTime());
  float incr = 1 / (float)numFrames;
  for (int i = 0; i < numFrames; ++i)
  {
    float perc = i * incr;
    if (((int)(i - _app->GetStartTime()) % (int)_fps) == 0)
    {
      pxr::GfVec2f p1(xmin * (1 - perc) + xmax * perc, ymin);
      pxr::GfVec2f p2(xmin * (1 - perc) + xmax * perc, ymid);
      drawList->AddLine(p1, p2, frameColor, 2);
    }
    else
    {
      pxr::GfVec2f p1(xmin * (1 - perc) + xmax * perc, ymin * 0.30 + ymid * 0.70);
      pxr::GfVec2f p2(xmin * (1 - perc) + xmax * perc, ymid);
      drawList->AddLine(p1, p2, frameColor, 2);
    }
  }

  // draw slider
  float sliderPerc =
    (float)(_app->GetCurrentTime() - _app->GetStartTime()) /
    (float)(_app->GetEndTime() - _app->GetStartTime());
  float sliderX = (xmin * (1 - sliderPerc) + xmax * sliderPerc);
  drawList->AddRectFilled(
    pxr::GfVec2f(sliderX - 2, ymin),
    pxr::GfVec2f(sliderX + 2, ymid),
    sliderColor, rounding, corners_all
  );
}



bool TimelineUI::Draw()
{
  bool changed = false;
  ImGui::Begin(_name.c_str(), NULL, _flags);

  ImGui::SetWindowPos(_parent->GetMin());
  ImGui::SetWindowSize(_parent->GetSize());

  DrawTimeSlider();
  DrawControls(&changed);

  ImGui::End();
  std::cout << "TIMELINE CHANGED : " << changed << std::endl;
  return changed;
}

AMN_NAMESPACE_CLOSE_SCOPE