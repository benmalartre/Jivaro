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
  Application* app = Application::Get();
  RegistryWindow::Get()->SetActiveTool(tool);
  RegistryWindow::Get()->GetMainWindow()->SetActiveTool(tool);
  SelectionChangedNotice().Send();
}

static void OnTranslateCallback()
{
  _SetActiveTool(Tool::TRANSLATE);
}

static void OnRotateCallback()
{
  _SetActiveTool(Tool::ROTATE);
}

static void OnScaleCallback()
{
  _SetActiveTool(Tool::SCALE);
}

static void OnSelectCallback()
{
  _SetActiveTool(Tool::SELECT);
}

static void OnBrushCallback()
{
  _SetActiveTool(Tool::BRUSH);
}

static void OnPlayCallback()
{
  _SetActiveTool(Tool::NONE);
  Application::Get()->GetModel()->ToggleExec();
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
  bool enabled, CALLBACK_FN func, const VtArray<VtValue> args)
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
  ImGui::PushFont(DEFAULT_FONT);
  bool clicked = false;
  if(toggable) {
    if (UI::AddCheckableIconButton(
      0, icon, enabled ? ICON_SELECTED : ICON_DEFAULT, func)) {
      enabled = 1 - enabled;
    }
  } else {
    clicked = UI::AddCheckableIconButton(
      0, icon, (window->GetActiveTool() == tool) ? ICON_SELECTED : ICON_DEFAULT, func);
  }
  ImGui::PopFont();

  if (tooltip.length() && ImGui::IsItemHovered()) {
    ui->AttachTooltip(tooltip.c_str());
  }
    
  return clicked;
}

int
ToolbarUIFixedSizeFunc(BaseUI* ui)
{
  return ((ToolbarUI*)ui)->GetFixedSize();
}

ToolbarUI::ToolbarUI(View* parent, bool vertical) 
  : BaseUI(parent, UIType::TOOLBAR) 
  , _vertical(vertical)
{
  ToolbarItem* selectItem = new ToolbarButton(
    this, Tool::SELECT, "Select", "Space","selection tool",
    ICON_FA_ARROW_POINTER, false, true,
    OnSelectCallback
  );
  _items.push_back(selectItem);

  ToolbarItem* translateItem = new ToolbarButton(
    this, Tool::TRANSLATE, "Translate", "T", "translation tool",
    ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT, false, true,
    OnTranslateCallback
  );
  _items.push_back(translateItem);

  ToolbarItem* rotateItem = new ToolbarButton(
    this, Tool::ROTATE, "Rotate", "R", "rotation tool",
    ICON_FA_ROTATE, false, true,
    OnRotateCallback
  );
  _items.push_back(rotateItem);

  ToolbarItem* scaleItem = new ToolbarButton(
    this, Tool::SCALE, "Scale", "S", "scale tool",
    ICON_FA_UP_RIGHT_AND_DOWN_LEFT_FROM_CENTER , false, true,
    OnScaleCallback
  );
  _items.push_back(scaleItem);

  ToolbarItem* brushItem = new ToolbarButton(
    this, Tool::BRUSH, "Brush", "B", "brush tool",
    ICON_FA_PAINTBRUSH, false, true,
    OnBrushCallback
  );
  _items.push_back(brushItem);

  ToolbarItem* playItem = new ToolbarButton(
    this, Tool::NONE, "Play", "play", "launch engine",
    ICON_FA_SHUFFLE, true, false,
    OnPlayCallback
  );
  _items.push_back(playItem);

  _parent->SetFixedSizeFunc(&ToolbarUIFixedSizeFunc);

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
      if (button->tool == Tool::NONE) continue;
      if (button->enabled && button->tool != window->GetActiveTool()) {
        button->enabled = false;
      }
    }
  }
}

int
ToolbarUI::GetFixedSize()
{
  const ImGuiStyle &style = ImGui::GetStyle(); 
  if(_vertical)return BUTTON_NORMAL_SIZE[0] + 2 * (style.WindowPadding.x + style.ItemSpacing.x);
  else return BUTTON_NORMAL_SIZE[1] + 2 * (style.WindowPadding.y + style.ItemSpacing.y);
}

bool ToolbarUI::Draw()
{
  bool opened;

  ImGui::SetNextWindowSize(_parent->GetSize());
  ImGui::SetNextWindowPos(_parent->GetMin());

  ImGui::Begin(_name.c_str(), &opened, _flags);
  

  ImGui::PushClipRect(
    _parent->GetMin(),
    _parent->GetMax(), 
    false);
 
  const ImGuiStyle& style = ImGui::GetStyle();
  ImGui::SetCursorPos(ImVec2(
    style.WindowPadding.x + style.ItemSpacing.x,
    style.WindowPadding.y + style.ItemSpacing.y
  ));
  for (auto& item : _items) {
    item->Draw();
    if(!_vertical) ImGui::SameLine();
    else ImGui::SetCursorPosX(style.WindowPadding.x + style.ItemSpacing.x);
  }
  ImGui::PopClipRect();
  ImGui::End();
  return ImGui::IsAnyItemActive() ||
    ImGui::IsAnyItemFocused() ||
    ImGui::IsAnyItemHovered() ||
    ImGui::IsAnyMouseDown();
};

JVR_NAMESPACE_CLOSE_SCOPE