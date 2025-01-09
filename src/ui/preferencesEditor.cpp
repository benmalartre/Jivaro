#include "../ui/preferences.h"


JVR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags DebugUI::_flags =
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoCollapse |
  ImGuiWindowFlags_NoNav |
  ImGuiWindowFlags_NoScrollWithMouse |
  ImGuiWindowFlags_NoScrollbar;

DebugUI::DebugUI(View* parent)
  : BaseUI(parent, UIType::DEBUG)
{
}

DebugUI::~DebugUI()
{
}


void 
DebugUI::_DrawTraceReporter() 
{

  static std::string reportStr;

  if (ImGui::Button("Start Tracing")) {
    TraceCollector::GetInstance().SetEnabled(true);
  }
  ImGui::SameLine();
  if (ImGui::Button("Stop Tracing")) {
    TraceCollector::GetInstance().SetEnabled(false);
  }
  ImGui::SameLine();
  if (ImGui::Button("Reset counters")) {
    std::ostringstream report;
    TraceReporter::GetGlobalReporter()->Report(report);
    reportStr = report.str();
  }
  ImGui::SameLine();

  if (ImGui::Button("Update tree")) {
    TraceReporter::GetGlobalReporter()->UpdateTraceTrees();
  }
  if (TraceCollector::IsEnabled()) {
    std::ostringstream report;
    
    reportStr = TraceCollector::GetInstance().GetLabel();
  }
  ImGuiIO& io = ImGui::GetIO();
  ImGui::PushFont(io.Fonts->Fonts[1]);
  const ImVec2 size(-FLT_MIN, -10);
  ImGui::InputTextMultiline("##TraceReport", &reportStr, size);
  ImGui::PopFont();
}

void 
DebugUI::_DrawDebugCodes() 
{
  // TfDebug::IsCompileTimeEnabled()
  ImVec2 listBoxSize(-FLT_MIN, -10);
  if (ImGui::BeginListBox("##DebugCodes", listBoxSize)) {
    for (auto& code : TfDebug::GetDebugSymbolNames()) {
      bool isEnabled = TfDebug::IsDebugSymbolNameEnabled(code);
      if (ImGui::Checkbox(code.c_str(), &isEnabled)) {
        TfDebug::SetDebugSymbolsByName(code, isEnabled);
      }
    }
    ImGui::EndListBox();
  }
}

// Draw a preference like panel
void DrawDebugUI() {
  
  
}

bool DebugUI::Draw()
{
  static const char* const panels[] = { 
    "Timings", 
    "Debug codes", 
    "Trace reporter"
  };
  static int current_item = 0;

  const GfVec2f min(GetX(), GetY());
  const GfVec2f size(GetWidth(), GetHeight());

  ImGui::SetNextWindowPos(min);
  ImGui::SetNextWindowSize(size);

  ImGui::Begin(_name.c_str(), NULL, _flags);
  

  ImGui::PushItemWidth(100);
  ImGui::ListBox("##DebugPanels", &current_item, panels, 3);
  ImGui::SameLine();
  if (current_item == 0) {
    ImGui::BeginChild("##Timing");
    ImGui::Text("ImGui: %.3f ms/frame  (%.1f FPS)", 
      1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::EndChild();
  }
  else if (current_item == 1) {
    ImGui::BeginChild("##DebugCodes");
    _DrawDebugCodes();
    ImGui::EndChild();
  }
  else if (current_item == 2) {
    ImGui::BeginChild("##TraceReporter");
    _DrawTraceReporter();
    ImGui::EndChild();
  }

  ImGui::End();
  return true;
};
  

JVR_NAMESPACE_CLOSE_SCOPE