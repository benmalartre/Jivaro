#include <pxr/usd/sdf/variantSpec.h>
#include <pxr/usd/sdf/variantSetSpec.h>
#include <pxr/usd/sdf/primSpec.h>
#include "../ui/utils.h"
#include "../ui/textEditor.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/commands.h"
#include "../app/application.h"

JVR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags TextEditorUI::_flags = 0
  | ImGuiWindowFlags_NoResize
  | ImGuiWindowFlags_NoTitleBar
  | ImGuiWindowFlags_NoMove;


TextEditorUI::TextEditorUI(View* parent)
  :BaseUI(parent, UIType::TEXTEDITOR)
{
}

TextEditorUI::~TextEditorUI()
{
}

bool 
TextEditorUI::Draw() 
{
  static std::string layerText;
  ImGuiIO &io = ImGui::GetIO();
  ImGuiWindow *window = ImGui::GetCurrentWindow();
  if (window->SkipItems) {
    return false;
  }
  const pxr::GfVec2f min(GetX(), GetY());
  const pxr::GfVec2f size(GetWidth(), GetHeight());

  ImGui::SetNextWindowPos(min);
  ImGui::SetNextWindowSize(size);

  ImGui::Begin(_name.c_str(), NULL, _flags);
  

  ImGui::Text("WARNING: this will slow down the application if the layer is big");
  ImGui::Text("        and will consume lots of memory. Use with care for now");
  _layer = Application::Get()->GetWorkStage()->GetRootLayer();
  if (_layer) {
    _layer->ExportToString(&layerText);
    ImGui::Text("%s", _layer->GetDisplayName().c_str());
  }
  ImGui::PushItemWidth(-FLT_MIN);
  ImGuiWindow *currentWindow = ImGui::GetCurrentWindow();
  ImVec2 sizeArg(0, currentWindow->Size[1] - 120);
  ImGui::PushFont(GetWindow()->GetFont(FONT_MEDIUM, 0));
  {
    //ScopedStyleColor color(ImGuiCol_FrameBg, ImVec4{0.0, 0.0, 0.0, 1.0});
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ 0.0, 0.0, 0.0, 1.0 });
    ImGui::InputTextMultiline("###TextEditor", &layerText, sizeArg,
                              ImGuiInputTextFlags_None | ImGuiInputTextFlags_NoUndoRedo);
    if (_layer && ImGui::IsItemDeactivatedAfterEdit()) {
      ADD_COMMAND(LayerTextEditCommand, _layer, layerText);
    }
    ImGui::PopStyleColor();
  }
  ImGui::PopFont();
  ImGui::PopItemWidth();
  ImGui::Text("Ctrl+Enter to apply your change");
  ImGui::End();

  return true;
}

JVR_NAMESPACE_CLOSE_SCOPE