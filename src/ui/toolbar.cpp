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
  app->SetActiveTool(tool);
  app->GetMainWindow()->SetActiveTool(tool);
  SelectionChangedNotice().Send();
}

static void OnTranslateCallback()
{
  _SetActiveTool(TOOL_TRANSLATE);
}

static void OnRotateCallback()
{
  _SetActiveTool(TOOL_ROTATE);
}

static void OnScaleCallback()
{
  _SetActiveTool(TOOL_SCALE);
}

static void OnSelectCallback()
{
  _SetActiveTool(TOOL_SELECT);
}

static void OnBrushCallback()
{
  _SetActiveTool(TOOL_BRUSH);
}

static void OnPlayCallback()
{
  _SetActiveTool(TOOL_NONE);
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
  const std::string& shortcut, const std::string& tooltip, const char* icon, bool toggable, 
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
  //ImGui::PushFont(window->GetRegularFont(0));
  bool clicked = false;
  if(toggable) {
    if (UIUtils::AddCheckableIconButton<UIUtils::CALLBACK_FN>(
      0, icon, enabled ? ICON_SELECTED : ICON_DEFAULT, func)) {
      enabled = 1 - enabled;
    }
  } else {
    clicked = UIUtils::AddCheckableIconButton<UIUtils::CALLBACK_FN>(
      0, icon, (window->GetActiveTool() == tool) ? ICON_SELECTED : ICON_DEFAULT, func);
  }
  //ImGui::PopFont();

  if (tooltip.length() && ImGui::IsItemHovered()) {
    ui->AttachTooltip(tooltip.c_str());
  }
    
  return clicked;
}

ToolbarUI::ToolbarUI(View* parent, bool vertical) 
  : BaseUI(parent, UIType::TOOLBAR) 
  , _vertical(vertical)
{
  ToolbarItem* selectItem = new ToolbarButton(
    this, TOOL_SELECT, "Select", "Space","selection tool",
    ICON_FA_ARROW_POINTER, false, true,
    (UIUtils::CALLBACK_FN)&OnSelectCallback
  );
  _items.push_back(selectItem);

  ToolbarItem* translateItem = new ToolbarButton(
    this, TOOL_TRANSLATE, "Translate", "T", "translation tool",
    ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT, false, true,
    (UIUtils::CALLBACK_FN)&OnTranslateCallback
  );
  _items.push_back(translateItem);

  ToolbarItem* rotateItem = new ToolbarButton(
    this, TOOL_ROTATE, "Rotate", "R", "rotation tool",
    ICON_FA_ROTATE, false, true,
    (UIUtils::CALLBACK_FN)&OnRotateCallback
  );
  _items.push_back(rotateItem);

  ToolbarItem* scaleItem = new ToolbarButton(
    this, TOOL_SCALE, "Scale", "S", "scale tool",
    ICON_FA_UP_RIGHT_AND_DOWN_LEFT_FROM_CENTER , false, true,
    (UIUtils::CALLBACK_FN)&OnScaleCallback
  );
  _items.push_back(scaleItem);

  ToolbarItem* brushItem = new ToolbarButton(
    this, TOOL_BRUSH, "Brush", "B", "brush tool",
    ICON_FA_PAINTBRUSH, false, true,
    (UIUtils::CALLBACK_FN)&OnBrushCallback
  );
  _items.push_back(brushItem);

  ToolbarItem* playItem = new ToolbarButton(
    this, TOOL_NONE, "Play", "play", "launch engine",
    ICON_FA_SHUFFLE, true, false,
    (UIUtils::CALLBACK_FN)&OnPlayCallback
  );
  _items.push_back(playItem);
}

ToolbarUI::~ToolbarUI() 
{
  for(auto& item: _items) delete item;
  _items.clear();
}

void ToolbarUI::Update()
{
  Window* window = GetView()->GetWindow();
  for (auto& item : this->_items) {
    if (item->type == TOOLBAR_BUTTON) {
      ToolbarButton* button = (ToolbarButton*)item;
      if (button->tool == TOOL_NONE) continue;
      if (button->enabled && button->tool != window->GetActiveTool()) {
        button->enabled = false;
      }
    }
  }
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