#include "toolbar.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/application.h"
#include "../app/modal.h"
#include "../ui/ui.h"
#include "../ui/style.h"

AMN_NAMESPACE_OPEN_SCOPE

static void _SetActiveTool(short tool) 
{
  Application* app = AMN_APPLICATION;
  app->GetTools()->SetActiveTool(tool);
  app->GetMainWindow()->SetActiveTool(tool);
}

static void OnOpenCallback()
{
  Application* app = AMN_APPLICATION;
  const char* folder = "/Users/benmalartre/Documents/RnD/amnesie/build";
  const char* filters[] = {
    ".usd",
    ".usda",
    ".usdc",
    ".usdz"
  };
  int numFilters = 4;

  std::string filename = 
    app->BrowseFile(folder, filters, numFilters, "open usd file");
  app->OpenScene(filename);

}

static void OnSaveCallback()
{
  std::cout << "ON SAVE CALLBACK !!!" << std::endl;
}

static void OnTranslateCallback()
{
  std::cout << "ON TRANSLATE CALLBACK!!!" << std::endl;
  _SetActiveTool(AMN_TOOL_TRANSLATE);
}

static void OnRotateCallback()
{
  std::cout << "ON ROTATE CALLBACK!!!" << std::endl;
  _SetActiveTool(AMN_TOOL_ROTATE);
}

static void OnScaleCallback()
{
  std::cout << "ON SCALE CALLBACK!!!" << std::endl;
  _SetActiveTool(AMN_TOOL_SCALE);
}

static void OnSelectCallback()
{
  std::cout << "ON SELECT CALLBACK!!!" << std::endl;
  _SetActiveTool(AMN_TOOL_SELECT);
}

ToolbarSeparator::ToolbarSeparator(BaseUI* ui, short orientation)
  : ToolbarItem(ui, TOOLBAR_SEPARATOR)
  , orientation(orientation)
{
}

bool ToolbarSeparator::Draw()
{
  return true;
}

ToolbarButton::ToolbarButton(BaseUI* ui, short tool, const std::string label, 
  const std::string shortcut, Icon* icon, bool toggable, bool enabled, 
  IconPressedFunc func, const pxr::VtArray<pxr::VtValue> args)
  : ToolbarItem(ui, TOOLBAR_BUTTON)
  , tool(tool)
  , label(label)
  , shortcut(shortcut)
  , icon(icon)
  , toggable(toggable)
  , enabled(enabled)
  , func(func)
  , args(args)
{
}

bool ToolbarButton::Draw()
{
  Window* window = ui->GetView()->GetWindow();
  ImGui::PushFont(window->GetRegularFont(0));
  
  if(toggable) {
    AddCheckableIconButton<IconPressedFunc>(
      icon,
      (window->GetActiveTool() == tool),
      func
    );
  } else {
    AddIconButton<IconPressedFunc>(
      icon,
      func
    );
  }
  ImGui::SameLine();

  ImGui::PopFont();
  return false;
}

ToolbarUI::ToolbarUI(View* parent, const std::string& name) :BaseUI(parent, name) 
{
  ToolbarItem* openItem = new ToolbarButton(
    this, AMN_TOOL_OPEN, "Open", "Ctrl+O",
    &AMN_ICONS[AMN_ICON_MEDIUM][ICON_OPEN], false, true, 
    (IconPressedFunc)&OnOpenCallback
  );
  _items.push_back(openItem);

  ToolbarItem* saveItem = new ToolbarButton(
    this, AMN_TOOL_SAVE, "Save", "Ctrl+S",
    &AMN_ICONS[AMN_ICON_MEDIUM][ICON_SAVE], false, true, 
    (IconPressedFunc)&OnSaveCallback
  );
  _items.push_back(saveItem);

  ToolbarItem* selectItem = new ToolbarButton(
    this, AMN_TOOL_SELECT, "Select", "Space",
    &AMN_ICONS[AMN_ICON_MEDIUM][ICON_SELECT], true, true, 
    (IconPressedFunc)&OnSelectCallback
  );
  _items.push_back(selectItem);

  ToolbarItem* translateItem = new ToolbarButton(
    this, AMN_TOOL_TRANSLATE, "Translate", "T", 
    &AMN_ICONS[AMN_ICON_MEDIUM][ICON_TRANSLATE], true, true, 
    (IconPressedFunc)&OnTranslateCallback
  );
  _items.push_back(translateItem);

  ToolbarItem* rotateItem = new ToolbarButton(
    this, AMN_TOOL_ROTATE, "Rotate", "R", 
    &AMN_ICONS[AMN_ICON_MEDIUM][ICON_ROTATE], true, true, 
    (IconPressedFunc)&OnRotateCallback
  );
  _items.push_back(rotateItem);

  ToolbarItem* scaleItem = new ToolbarButton(
    this, AMN_TOOL_SCALE, "Scale", "S", 
    &AMN_ICONS[AMN_ICON_MEDIUM][ICON_SCALE], true, true, 
    (IconPressedFunc)&OnScaleCallback
  );
  _items.push_back(scaleItem);
}

ToolbarUI::~ToolbarUI() 
{
  for(auto& item: _items) delete item;
  _items.clear();
}

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
  for (auto& item : _items)item->Draw();
  ImGui::PopClipRect();
  ImGui::End();
  return ImGui::IsAnyItemActive() ||
    ImGui::IsAnyItemFocused() ||
    ImGui::IsAnyItemHovered() ||
    ImGui::IsAnyMouseDown();
};

AMN_NAMESPACE_CLOSE_SCOPE