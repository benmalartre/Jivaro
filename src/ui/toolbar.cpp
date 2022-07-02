#include "../ui/ui.h"
#include "../ui/style.h"
#include "../ui/toolbar.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/application.h"
#include "../app/modal.h"

JVR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags ToolbarUI::_flags = 
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoScrollbar |
  ImGuiWindowFlags_NoDecoration;

static void _SetActiveTool(short tool) 
{
  Application* app = APPLICATION;
  app->GetTools()->SetActiveTool(tool);
  app->GetMainWindow()->SetActiveTool(tool);
  SelectionChangedNotice().Send();
}

static void OnTranslateCallback()
{
  std::cout << "ON TRANSLATE CALLBACK!!!" << std::endl;
  _SetActiveTool(TOOL_TRANSLATE);
}

static void OnRotateCallback()
{
  std::cout << "ON ROTATE CALLBACK!!!" << std::endl;
  _SetActiveTool(TOOL_ROTATE);
}

static void OnScaleCallback()
{
  std::cout << "ON SCALE CALLBACK!!!" << std::endl;
  _SetActiveTool(TOOL_SCALE);
}

static void OnSelectCallback()
{
  std::cout << "ON SELECT CALLBACK!!!" << std::endl;
  _SetActiveTool(TOOL_SELECT);
}

static void OnBrushCallback()
{
  std::cout << "ON BRUSH CALLBACK!!!" << std::endl;
  _SetActiveTool(TOOL_BRUSH);
  std::cout << "ACTIVE TOOL : BRUSH" << std::endl;
}

static void OnPlayCallback()
{
  std::cout << "ON PLAY CALLBACK!!!" << std::endl;
  GetApplication()->ToggleExec();
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

ToolbarButton::ToolbarButton(BaseUI* ui, short tool, const std::string& label, 
  const std::string& shortcut, const std::string& tooltip, Icon* icon, bool toggable, 
  bool enabled, UIUtils::CALLBACK_FN func, const pxr::VtArray<pxr::VtValue> args)
  : ToolbarItem(ui, TOOLBAR_BUTTON)
  , tool(tool)
  , label(label)
  , shortcut(shortcut)
  , tooltip(tooltip)
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
    UIUtils::AddCheckableIconButton<UIUtils::CALLBACK_FN>(
      0, icon, (window->GetActiveTool() == tool) ? ICON_SELECTED : ICON_DEFAULT, func);
  } else {
    UIUtils::AddIconButton<UIUtils::CALLBACK_FN>(
      1, icon, ICON_DEFAULT, func);
  }
  ImGui::PopFont();

  if (tooltip.length() && ImGui::IsItemHovered()) {
    ui->AttachTooltip(tooltip.c_str());
  }
    
 
  return false;
}

ToolbarUI::ToolbarUI(View* parent, const std::string& name, bool vertical) 
  : BaseUI(parent, name) 
  , _vertical(vertical)
{
  /*
  ToolbarItem* openItem = new ToolbarButton(
    this, TOOL_OPEN, "Open", "Ctrl+O",
    &ICONS[ICON_MEDIUM][ICON_OPEN], false, true, 
    (UIUtils::CALLBACK_FN)&OnOpenCallback
  );
  _items.push_back(openItem);

  ToolbarItem* saveItem = new ToolbarButton(
    this, TOOL_SAVE, "Save", "Ctrl+S",
    &ICONS[ICON_MEDIUM][ICON_SAVE], false, true, 
    (UIUtils::CALLBACK_FN)&OnSaveCallback
  );
  _items.push_back(saveItem);
  */
  ToolbarItem* selectItem = new ToolbarButton(
    this, TOOL_SELECT, "Select", "Space","selection tool",
    &ICONS[ICON_SIZE_MEDIUM][ICON_SELECT], true, true, 
    (UIUtils::CALLBACK_FN)&OnSelectCallback
  );
  _items.push_back(selectItem);

  ToolbarItem* translateItem = new ToolbarButton(
    this, TOOL_TRANSLATE, "Translate", "T", "translation tool",
    &ICONS[ICON_SIZE_MEDIUM][ICON_TRANSLATE], true, true, 
    (UIUtils::CALLBACK_FN)&OnTranslateCallback
  );
  _items.push_back(translateItem);

  ToolbarItem* rotateItem = new ToolbarButton(
    this, TOOL_ROTATE, "Rotate", "R", "rotation tool",
    &ICONS[ICON_SIZE_MEDIUM][ICON_ROTATE], true, true, 
    (UIUtils::CALLBACK_FN)&OnRotateCallback
  );
  _items.push_back(rotateItem);

  ToolbarItem* scaleItem = new ToolbarButton(
    this, TOOL_SCALE, "Scale", "S", "scale tool",
    &ICONS[ICON_SIZE_MEDIUM][ICON_SCALE], true, true, 
    (UIUtils::CALLBACK_FN)&OnScaleCallback
  );
  _items.push_back(scaleItem);

  ToolbarItem* brushItem = new ToolbarButton(
    this, TOOL_BRUSH, "Brush", "B", "brush tool",
    &ICONS[ICON_SIZE_MEDIUM][ICON_BRUSH], true, true, 
    (UIUtils::CALLBACK_FN)&OnBrushCallback
  );
  _items.push_back(brushItem);

  ToolbarItem* playItem = new ToolbarButton(
    this, TOOL_NONE, "Play", "play", "launch engine",
    &ICONS[ICON_SIZE_MEDIUM][ICON_PLAYBACK_FORWARD], true, true,
    (UIUtils::CALLBACK_FN)&OnPlayCallback
  );
  _items.push_back(playItem);
}

ToolbarUI::~ToolbarUI() 
{
  for(auto& item: _items) delete item;
  _items.clear();
}

bool ToolbarUI::Draw()
{
  bool opened;
  ImGui::Begin(_name.c_str(), &opened, _flags);
  ImGui::SetWindowSize(_parent->GetSize());
  ImGui::SetWindowPos(_parent->GetMin());

  ImGui::PushClipRect(
    _parent->GetMin(),
    _parent->GetMax(), 
    false);
 
  ImGui::SetCursorPosY(4.f);
  for (auto& item : _items) {
    item->Draw();
    if(!_vertical) ImGui::SameLine();
  }
  ImGui::PopClipRect();
  ImGui::End();
  return ImGui::IsAnyItemActive() ||
    ImGui::IsAnyItemFocused() ||
    ImGui::IsAnyItemHovered() ||
    ImGui::IsAnyMouseDown();
};

JVR_NAMESPACE_CLOSE_SCOPE