#include <functional>
#include "../utils/icons.h"
#include "../ui/timeline.h"
#include "../app/application.h"
#include "../app/time.h"
#include "../app/view.h"


JVR_NAMESPACE_OPEN_SCOPE

static void
PlaybackCallback(TimelineUI* ui)
{
  Time& time = GetApplication()->GetTime();
  TimeData& data = ui->GetData();
  data.playing = 1 - data.playing;
  if (data.playing) time.StartPlayBack();
  else time.StopPlayBack();
}

static void
PreviousFrameCallback(TimelineUI* ui)
{
  Time& time = GetApplication()->GetTime();
  time.PreviousFrame();
}

static void
NextFrameCallback(TimelineUI* ui)
{
  Time& time = GetApplication()->GetTime();
  time.NextFrame();
}

static void
FirstFrameCallback(TimelineUI* ui)
{
  Time& time = GetApplication()->GetTime();
  TimeData& data = ui->GetData();
  data.currentTime = data.startTime;
  time.SetActiveTime(data.currentTime);
}

static void
LastFrameCallback(TimelineUI* ui)
{
  Time& time = GetApplication()->GetTime();
  TimeData& data = ui->GetData();
  data.currentTime = data.endTime;
  time.SetActiveTime(data.currentTime);
}

static void
LoopCallback(TimelineUI* ui)
{
  TimeData& data = ui->GetData();
  data.loop = 1 - data.loop;
  Application* app = GetApplication();
  Time& time = app->GetTime();
  time.SetLoop(data.loop);
}

static void
SetFrameCallback(TimelineUI* ui)
{
  Time& time = GetApplication()->GetTime();
  TimeData& data = ui->GetData();
  time.SetActiveTime(data.currentTime);
}


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
  _data.minTime = time.GetMinTime();
  _data.startTime = time.GetStartTime();
  _data.endTime = time.GetEndTime();
  _data.maxTime = time.GetMaxTime();
  _data.currentTime = time.GetActiveTime();
  _data.loop = time.GetLoop();
  _data.fps = time.GetFPS();
  _data.playing = time.IsPlaying();
  _parent->SetDirty();
}

void 
TimelineUI::ValidateTime()
{
  Time& time = GetApplication()->GetTime();

  _data.currentTime = time.GetActiveTime();
  if (_data.minTime >= _data.maxTime)_data.maxTime = _data.minTime + 1;
  if (_data.endTime > _data.maxTime)_data.maxTime = _data.endTime;
  if (_data.startTime < _data.minTime)_data.startTime = _data.minTime;
  if (_data.endTime <= _data.startTime)_data.endTime = _data.startTime + 1;
  if (_data.currentTime < _data.startTime)_data.currentTime = _data.startTime;
  else if (_data.currentTime > _data.endTime)_data.currentTime = _data.endTime;
  
  time.SetMinTime(_data.minTime);
  time.SetStartTime(_data.startTime);
  time.SetEndTime(_data.endTime);
  time.SetMaxTime(_data.maxTime);
  time.SetActiveTime(_data.currentTime);
  time.SetLoop(_data.loop);
}

float 
TimelineUI::_GetFrameUnderMouse(int x, int y)
{
  View* parent = GetView();
  return RESCALE(x - parent->GetX(), 0.f, parent->GetWidth(), _data.minTime, _data.maxTime);
}

pxr::GfVec2f 
TimelineUI::_TimeToPosition(float time)
{
  return pxr::GfVec2f(RESCALE(time, _data.minTime, _data.maxTime, 
    0.f, _parent->GetWidth()), 0.f);
}

float 
TimelineUI::_PositionToTime(const pxr::GfVec2f& position)
{
  return RESCALE(position[0] - _parent->GetX(), 0.f, 
    _parent->GetWidth(), _data.minTime, _data.maxTime);
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
    _data.currentTime = _frame;
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
      _data.currentTime = _frame;
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
  UIUtils::AddIconButton<UIUtils::CALLBACK_FN>(
    0, ICON_FA_BACKWARD_FAST , ICON_DEFAULT,
    (UIUtils::CALLBACK_FN)FirstFrameCallback, this
    );
  ImGui::SameLine();

  UIUtils::AddIconButton<UIUtils::CALLBACK_FN>(
    1, ICON_FA_BACKWARD_STEP, ICON_DEFAULT,
    (UIUtils::CALLBACK_FN)PreviousFrameCallback, this
    );
  ImGui::SameLine();

  if (!_data.playing) {
    UIUtils::AddCheckableIconButton<UIUtils::CALLBACK_FN, TimelineUI*>(
      2, ICON_FA_PLAY , ICON_DEFAULT,
      (UIUtils::CALLBACK_FN)PlaybackCallback, this);
  } else {
    UIUtils::AddCheckableIconButton<UIUtils::CALLBACK_FN, TimelineUI*>(
      2, ICON_FA_STOP , ICON_SELECTED,
      (UIUtils::CALLBACK_FN)PlaybackCallback, this);
  }
  ImGui::SameLine();

  UIUtils::AddIconButton<UIUtils::CALLBACK_FN>(
    3, ICON_FA_FORWARD_STEP, ICON_DEFAULT,
    (UIUtils::CALLBACK_FN)NextFrameCallback, this);
  ImGui::SameLine();

  UIUtils::AddIconButton<UIUtils::CALLBACK_FN, TimelineUI*>(
    4, ICON_FA_FORWARD_FAST, ICON_DEFAULT,
    (UIUtils::CALLBACK_FN)LastFrameCallback, this);
  ImGui::SameLine();

  UIUtils::AddCheckableIconButton<UIUtils::CALLBACK_FN, TimelineUI*>(
    5, ICON_FA_ROTATE,
    _data.loop ? ICON_SELECTED : ICON_DEFAULT,
    (UIUtils::CALLBACK_FN)LoopCallback, this);
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
  ImGui::InputScalar("##minTime", ImGuiDataType_Float, &_data.minTime,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if (!ImGui::IsItemActive() && _data.minTime != app->GetTime().GetMinTime())
  {
    ValidateTime();
  }
  if (ImGui::IsItemHovered())
    AttachTooltip("Minimum Time");
  //AttachTooltip("Minimum Time", 0.5f, 128, GetWindow()->GetRegularFont(0));
  ImGui::SameLine(); 

  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##startTime", ImGuiDataType_Float, &_data.startTime,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if (!ImGui::IsItemActive() && _data.startTime != app->GetTime().GetStartTime())
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
  ImGui::InputScalar("##currentTime", ImGuiDataType_Float, &_data.currentTime,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if (!ImGui::IsItemActive() && _data.currentTime != app->GetTime().GetActiveTime())
  {
    ValidateTime();
  }
  if (ImGui::IsItemHovered())
    AttachTooltip("Current Time");
  //AttachTooltip("Current Time", 0.5f, 128, GetWindow()->GetRegularFont(0));
  ImGui::SameLine(); 

  ImGui::SetCursorPosX(width - 140);

  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##endTime", ImGuiDataType_Float, &_data.endTime,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if (!ImGui::IsItemActive() && _data.endTime != app->GetTime().GetEndTime())
  {
    ValidateTime();
  }
  ImGui::SameLine();

  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##maxTime", ImGuiDataType_Float, &_data.maxTime,
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  if (!ImGui::IsItemActive() && _data.maxTime != app->GetTime().GetMaxTime())
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

  int numFrames = (app->GetTime().GetEndTime() - app->GetTime().GetStartTime());
  float incr = 1 / (float)numFrames;
  for (int i = 0; i < numFrames; ++i)
  {
    float perc = i * incr;
    if (((int)(i - app->GetTime().GetStartTime()) % (int)_data.fps) == 0)
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
  const pxr::GfVec2f framePosition(_TimeToPosition(_frame));
  drawList->AddRectFilled(
    pxr::GfVec2f(framePosition[0]- 2.f, 0.f),
    pxr::GfVec2f(framePosition[0] + 2.f, ymid),
    ImColor(255, 0, 0, 255), rounding, corners_none);
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

JVR_NAMESPACE_CLOSE_SCOPE