#include "timeline.h"
#include "../app/application.h"

AMN_NAMESPACE_OPEN_SCOPE

// constructor
TimelineUI::TimelineUI(View* parent):BaseUI(parent, "Timeline")
{
  _flags = 0;
  _flags |= ImGuiWindowFlags_NoResize;
  _flags |= ImGuiWindowFlags_NoTitleBar;
  _flags |= ImGuiWindowFlags_NoMove;

  _minTime = 1;
  _startTime = 1;
  _maxTime = 120;
  _endTime = 120;
  _currentTime = 25;
}

// destructor
TimelineUI::~TimelineUI(){}

void TimelineUI::MouseButton(int action, int button, int mods)
{
  std::cout << "TIMELINE : MOUSE BUTTON :D" << std::endl;
}

void TimelineUI::MouseMove(int x, int y)
{
  std::cout << "TIMELINE : MOUSE MOVE :D" << std::endl;
}

int TimelineUI::StartFrameChanged(ImGuiTextEditCallbackData* data)
{

  std::cout << "Timeline Start Frame Changed!!!" << data->Buf << std::endl;
  /*
  std::string sTime(time);
  _startTime=std::stof(sTime);
  */
  /*
  if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
  {
      ImVector<char>* my_str = (ImVector<char>*)data->UserData;
      IM_ASSERT(my_str->begin() == data->Buf);
      my_str->resize(data->BufSize);  // NB: On resizing calls, generally data->BufSize == data->BufTextLen + 1
      data->Buf = my_str->begin();
  }
  */
  return 0;
}

void TimelineUI::Draw()
{
  /*
  float _currentTime;
  float _startTime;
  float _endTime;
  float _minTime;
  float _maxTime;
  float _fps;
  float _speed;
  bool  _loop;
  */
  
  static bool opened;
  ImGui::Begin(_name.c_str(), &opened, _flags);

  int width = GetWidth();
  int height = GetHeight();

  ImGui::SetCursorPosX(20);
  ImGui::SetCursorPosY(height-20);

  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##minTime", ImGuiDataType_Float, &_minTime, 
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  ImGui::SameLine();

  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##startTime", ImGuiDataType_Float, &_startTime, 
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  ImGui::SameLine();


  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##currentTime", ImGuiDataType_Float, &_currentTime, 
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  ImGui::SameLine();

  ImGui::Separator();
  ImGui::SameLine();

  ImGui::SetCursorPosX(width-140);

  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##endTime", ImGuiDataType_Float, &_endTime, 
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  ImGui::SameLine();

  ImGui::SetNextItemWidth(60);
  ImGui::InputScalar("##maxTime", ImGuiDataType_Float, &_maxTime, 
    NULL, NULL, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  ImGui::SameLine();

  //ImGui::InputFloat("##minTime", &_minTime, 1, 10, "%.3f", ImGuiInputTextFlags_AutoSelectAll);
  /*
  ImGui::InputText("##minTime", NULL, 0,
    ImGuiInputTextFlags_AutoSelectAll|ImGuiInputTextFlags_CharsDecimal|ImGuiInputTextFlags_CallbackAlways);
  ImGui::SameLine();

  ImGui::SetNextItemWidth(100);
  ImGui::InputText("##startTime", NULL, 0,
    ImGuiInputTextFlags_AutoSelectAll|ImGuiInputTextFlags_CharsDecimal|ImGuiInputTextFlags_CallbackAlways);
  ImGui::SameLine();

  ImGui::SetNextItemWidth(100);
  ImGui::InputText("##currentTime", NULL, 0,
    ImGuiInputTextFlags_AutoSelectAll|ImGuiInputTextFlags_CharsDecimal|ImGuiInputTextFlags_CallbackAlways);
  ImGui::SameLine();

  ImGui::SetNextItemWidth(100);
  ImGui::InputText("##endTime", NULL, 0, 
    ImGuiInputTextFlags_AutoSelectAll|ImGuiInputTextFlags_CharsDecimal|ImGuiInputTextFlags_CallbackAlways);
  ImGui::SameLine();

  ImGui::SetNextItemWidth(100);
  ImGui::InputText("##maxTime", NULL, 0,
    ImGuiInputTextFlags_AutoSelectAll|ImGuiInputTextFlags_CharsDecimal|ImGuiInputTextFlags_CallbackAlways);
  ImGui::SameLine();
*/
  ImGui::SetWindowPos(_parent->GetMin());
  ImGui::SetWindowSize(_parent->GetSize());
  ImGui::End();
}

AMN_NAMESPACE_CLOSE_SCOPE
