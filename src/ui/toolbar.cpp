#include "toolbar.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../ui/ui.h"
#include "../ui/style.h"
#include "../ui/icon.h"

AMN_NAMESPACE_OPEN_SCOPE

static void OnTranslateCallback()
{
  std::cout << "ON TRANSLATE CALLBACK!!!" << std::endl;
}

static void OnRotateCallback()
{
  std::cout << "ON ROTATE CALLBACK!!!" << std::endl;
}

static void OnScaleCallback()
{
  std::cout << "ON SCALE CALLBACK!!!" << std::endl;
}

static void OnSelectCallback()
{
  std::cout << "ON SELECT CALLBACK!!!" << std::endl;
}

ToolbarItem::ToolbarItem(BaseUI* ui, const std::string label, const std::string shortcut, Icon* icon,
  bool toggable, bool enabled, IconPressedFunc func, const pxr::VtArray<pxr::VtValue> args)
  : ui(ui)
  , label(label)
  , shortcut(shortcut)
  , icon(icon)
  , toggable(toggable)
  , enabled(enabled)
  , func(func)
  , args(args)
{

}

bool ToolbarItem::Draw()
{
  Window* window = ui->GetView()->GetWindow();
  
  ImGui::PushFont(window->GetRegularFont(0));
  
  AddIconButton<IconPressedFunc>(
    icon,
    func
    );
  ImGui::SameLine();

  ImGui::PopFont();
  return false;
}

ToolbarUI::ToolbarUI(View* parent, const std::string& name) :BaseUI(parent, name) 
{
  ToolbarItem selectItem(this, "Select", "Space", &AMN_ICONS[AMN_ICON_MEDIUM]["select.png"], false,
    true, (IconPressedFunc)&OnSelectCallback);
  _items.push_back(selectItem);

  ToolbarItem translateItem(this, "Translate", "T", &AMN_ICONS[AMN_ICON_MEDIUM]["translate.png"], false,
    true, (IconPressedFunc)&OnTranslateCallback);
  _items.push_back(translateItem);

  ToolbarItem rotateItem(this, "Rotate", "R", &AMN_ICONS[AMN_ICON_MEDIUM]["rotate.png"], false,
    true, (IconPressedFunc)&OnRotateCallback);
  _items.push_back(rotateItem);

  ToolbarItem scaleItem(this, "Scale", "S", &AMN_ICONS[AMN_ICON_MEDIUM]["scale.png"], false,
    true, (IconPressedFunc)&OnScaleCallback);
  _items.push_back(scaleItem);
}

ToolbarUI::~ToolbarUI() {}

bool ToolbarUI::Draw()
{
  bool opened;
  int flags = ImGuiWindowFlags_None 
    | ImGuiWindowFlags_NoResize
    | ImGuiWindowFlags_NoTitleBar
    | ImGuiWindowFlags_NoMove
    | ImGuiWindowFlags_NoScrollbar
    | ImGuiWindowFlags_NoDecoration;

  ImGui::Begin(_name.c_str(), &opened, flags);
  ImGui::PushClipRect(
    _parent->GetMin(),
    _parent->GetMax(), 
    false);

  ImGui::SetWindowSize(_parent->GetSize());
  ImGui::SetWindowPos(_parent->GetMin());
 
  ImGui::SetCursorPosY(4.f);
  for (auto& item : _items)item.Draw();
  ImGui::PopClipRect();
  ImGui::End();
  return true;
  return ImGui::IsAnyItemActive() ||
    ImGui::IsAnyItemFocused() ||
    ImGui::IsAnyItemHovered() ||
    ImGui::IsAnyMouseDown();
};

AMN_NAMESPACE_CLOSE_SCOPE