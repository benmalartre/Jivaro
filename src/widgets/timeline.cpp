#include "timeline.h"
#include "../app/application.h"
#include "../utils/icons.h"

AMN_NAMESPACE_OPEN_SCOPE

// constructor
TimelineUI::TimelineUI(View* parent):BaseUI(parent, "Timeline")
{
  _flags = 0;
  _flags |= ImGuiWindowFlags_NoResize;
  _flags |= ImGuiWindowFlags_NoTitleBar;
  _flags |= ImGuiWindowFlags_NoMove;  

  InitializeIcons();
}

// destructor
TimelineUI::~TimelineUI(){}

void TimelineUI::Init(Application* app)
{
  _app = app;
  Update();
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
}

void TimelineUI::ValidateTime()
{
  if(_minTime>=_maxTime)_maxTime = _minTime + 1;
  if(_endTime>_maxTime)_endTime = _maxTime;
  if(_startTime < _minTime)_startTime = _minTime;
  if(_endTime <= _startTime)_endTime = _startTime + 1;
  if(_maxTime <= _endTime)_maxTime = _endTime;
  if(_currentTime < _startTime)_currentTime = _startTime;
  else if(_currentTime > _endTime)_currentTime = _endTime;

  if(_app)
  {
    _app->SetMinTime(_minTime);
    _app->SetStartTime(_startTime);
    _app->SetEndTime(_endTime);
    _app->SetMaxTime(_maxTime);
    _app->SetCurrentTime(_currentTime);
  }
}

void TimelineUI::MouseButton(int action, int button, int mods)
{
  std::cout << "TIMELINE : MOUSE BUTTON :D" << std::endl;
}

void TimelineUI::MouseMove(int x, int y)
{
  std::cout << "TIMELINE : MOUSE MOVE :D" << std::endl;
}

void TimelineUI::DrawControls()
{
  int width = GetWidth();
  int height = GetHeight();

  ImGui::SetCursorPosX(20);
  ImGui::SetCursorPosY(height-20);

  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##minTime", ImGuiDataType_Float, &_minTime, 
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if(!ImGui::IsItemActive() && _minTime != _app->GetMinTime())
  {
    ValidateTime();
  }
  ImGui::SameLine();

  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##startTime", ImGuiDataType_Float, &_startTime, 
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if(!ImGui::IsItemActive() && _startTime != _app->GetStartTime())
  {
    ValidateTime();
  }
  ImGui::SameLine();
  
  Icon* icon = &AMN_ICONS["play_btn.png"];

  /*
  ImGui::PushFont(GetWindow()->GetRegularFont());
  ImGui::Text("pointer = %p", icon->_tex);
  ImGui::SameLine();
  ImGui::Text("size = %d x %d", icon->_size, icon->_size);
  ImGui::SameLine();
  ImGui::PopFont();
  */
  if (ImGui::ImageButton(
      (void *)icon->_tex, 
      ImVec2(icon->_size,icon->_size), 
      ImVec2(0,0), 
      ImVec2(0, 1), 
      -2, 
      ImVec4(1.0f,0.0f,0.0f,0.0f),
      ImVec4(1.0f,1.0f,1.0f,1.0f)))
  {
    std::cout << "MY FUCKIN ICON WAS CLICKED!!!" << std::endl;
  }
  ImGui::SameLine();

  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##currentTime", ImGuiDataType_Float, &_currentTime, 
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if(!ImGui::IsItemActive() && _currentTime != _app->GetCurrentTime())
  {
    ValidateTime();
  }
  ImGui::SameLine();

  ImGui::SetCursorPosX(width-140);

  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##endTime", ImGuiDataType_Float, &_endTime, 
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if(!ImGui::IsItemActive() && _endTime != _app->GetEndTime())
  {
    ValidateTime();
  }
  ImGui::SameLine();

  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##maxTime", ImGuiDataType_Float, &_maxTime, 
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if(!ImGui::IsItemActive() && _maxTime != _app->GetMaxTime())
  {
    ValidateTime();
  }
  ImGui::SameLine();
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
  static ImColor sliderColor(colors[ImGuiCol_PlotHistogram]);
  static ImColor frameColor(colors[ImGuiCol_PlotLines]);

  // draw background
  drawList->AddRectFilled(
    pxr::GfVec2f(xmin, ymin), 
    pxr::GfVec2f(xmax, ymax), 
    backColor, rounding,  corners_none
  ); 

  // draw frames
  drawList->AddRectFilled(
    pxr::GfVec2f(xmin, ymid), 
    pxr::GfVec2f(xmax, ymax), 
    frontColor, rounding,  corners_none);

  int numFrames = (_app->GetEndTime() - _app->GetStartTime());
  float incr = 1 /(float) numFrames;
  for(int i=0;i<numFrames;++i)
  {
    float perc = i * incr;
    if(((int)(i-_app->GetStartTime()) % (int)_fps) == 0)
    {
      pxr::GfVec2f p1(xmin * (1-perc) + xmax * perc, ymin);
      pxr::GfVec2f p2(xmin * (1-perc) + xmax * perc, ymid);
      drawList->AddLine(p1, p2, frameColor, 2);
    }
    else
    {
      pxr::GfVec2f p1(xmin * (1-perc) + xmax * perc, ymin * 0.30 + ymid * 0.70);
      pxr::GfVec2f p2(xmin * (1-perc) + xmax * perc, ymid);
      drawList->AddLine(p1, p2, frameColor, 2);
    }
  }
  // draw slider
  float sliderPerc = 
    (float)(_app->GetCurrentTime() - _app->GetStartTime()) / 
    (float)(_app->GetEndTime() - _app->GetStartTime());
  float sliderX = (xmin * (1 - sliderPerc) + xmax *sliderPerc);
  drawList->AddRectFilled(
    pxr::GfVec2f(sliderX - 2, ymin), 
    pxr::GfVec2f(sliderX + 2, ymid), 
    sliderColor, rounding,  corners_all
  ); 
}

void TimelineUI::Draw()
{

  ImGui::Begin(_name.c_str(), NULL, _flags);
  ImGui::SetWindowPos(_parent->GetMin());
  ImGui::SetWindowSize(_parent->GetSize());

  DrawTimeSlider();
  DrawControls();
  
  ImGui::End();
}

AMN_NAMESPACE_CLOSE_SCOPE
