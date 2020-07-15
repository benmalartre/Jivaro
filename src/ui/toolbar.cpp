#include "toolbar.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/ui.h"
#include "../widgets/icon.h"

AMN_NAMESPACE_OPEN_SCOPE

static void OnTranslateCallback()
{
  std::cout << "ON TRANSLATE CALLBACK!!!" << std::endl;
}

ToolbarItem::ToolbarItem(BaseUI* ui, const std::string label, const std::string shortcut, bool selected,
  bool enabled, ToolbarPressedFunc func, const pxr::VtArray<pxr::VtValue> args)
  : ui(ui), label(label), shortcut(shortcut), selected(selected), enabled(enabled), func(func), args(args)
{

}

bool ToolbarItem::Draw()
{
  Window* window = ui->GetView()->GetWindow();
  ImGui::SetCursorPos(ImVec2(4, 4));

  /*
  const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO", "PPPP", "QQQQQQQQQQ", "RRR", "SSSS" };
  static const char* current_item = NULL;

  ImGui::PushItemWidth(120);
  if (ImGui::BeginCombo("##combo", current_item)) // The second parameter is the label previewed before opening the combo.
  {
    for (int n = 0; n < IM_ARRAYSIZE(items); n++)
    {
      bool is_selected = (current_item == items[n]); // You can store your selection however you want, outside or inside your objects
      if (ImGui::Selectable(items[n], is_selected))
        current_item = items[n];
      if (is_selected)
        ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
    }
    ImGui::EndCombo();
  }
  ImGui::PopItemWidth();

  ImGui::SameLine();
  */ 
  ImGui::PushFont(window->GetRegularFont(0));
  Icon* icon = &AMN_ICONS[AMN_ICON_MEDIUM]["select.png"];
  AddIconButton<IconPressedFunc>(
    icon,
    (IconPressedFunc)OnTranslateCallback
    );
  ImGui::SameLine();
  
  icon = &AMN_ICONS[AMN_ICON_MEDIUM]["rotate.png"];
  AddIconButton<IconPressedFunc>(
    icon,
    (IconPressedFunc)OnTranslateCallback
    );

  ImGui::SameLine();
  icon = &AMN_ICONS[AMN_ICON_MEDIUM]["translate.png"];
  AddIconButton<IconPressedFunc>(
    icon,
    (IconPressedFunc)OnTranslateCallback
    );

  ImGui::SameLine();
  icon = &AMN_ICONS[AMN_ICON_MEDIUM]["scale.png"];
  AddIconButton<IconPressedFunc>(
    icon,
    (IconPressedFunc)OnTranslateCallback
    );

  ImGui::PopFont();
  return false;
}

ToolbarUI::ToolbarUI(View* parent, const std::string& name) :BaseUI(parent, name) 
{
  ToolbarItem item(this, "Translate", "T", false,
    true, (ToolbarPressedFunc)&OnTranslateCallback);
  _items.push_back(item);
}

ToolbarUI::~ToolbarUI() {}

bool ToolbarUI::Draw()
{
  bool opened;
  int flags = ImGuiWindowFlags_None 
    | ImGuiWindowFlags_NoResize
    | ImGuiWindowFlags_NoTitleBar
    | ImGuiWindowFlags_NoMove
    | ImGuiWindowFlags_NoScrollbar;

  ImGui::Begin(_name.c_str(), &opened, flags);
  pxr::GfVec4f color(
    RANDOM_0_1,
    RANDOM_0_1,
    RANDOM_0_1,
    1.f
  );

  ImGui::SetWindowSize(_parent->GetSize());
  ImGui::SetWindowPos(_parent->GetMin());
 
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  for (auto& item : _items)item.Draw();

  ImGui::End();

  return ImGui::IsAnyItemActive() ||
    ImGui::IsAnyItemFocused() ||
    ImGui::IsAnyItemHovered() ||
    ImGui::IsAnyMouseDown();
};

AMN_NAMESPACE_CLOSE_SCOPE