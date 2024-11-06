#include "../ui/toolbar.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/registry.h"
#include "../app/callbacks.h"
#include "../app/model.h"
#include "../app/modal.h"

JVR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags ToolbarUI::_flags = 
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoScrollbar |
  ImGuiWindowFlags_NoDecoration;



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
      0, icon, enabled ? UI::STATE_SELECTED : UI::STATE_DEFAULT, func)) {
      enabled = 1 - enabled;
    }
  } else {
    clicked = UI::AddCheckableIconButton(
      0, icon, (window->GetActiveTool() == tool) ? UI::STATE_SELECTED : UI::STATE_DEFAULT, func);
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
  ToolbarButton* selectItem = new ToolbarButton(
    this, Tool::SELECT, "Select", "Space","selection tool",
    ICON_FA_ARROW_POINTER, true, true,
    std::bind(Callbacks::SetActiveTool, Tool::SELECT)
  );
  _items.push_back(selectItem);

  ToolbarButton* translateItem = new ToolbarButton(
    this, Tool::TRANSLATE, "Translate", "T", "translation tool",
    ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT, true, false,
    std::bind(Callbacks::SetActiveTool, Tool::TRANSLATE)
  );
  _items.push_back(translateItem);

  ToolbarButton* rotateItem = new ToolbarButton(
    this, Tool::ROTATE, "Rotate", "R", "rotation tool",
    ICON_FA_ROTATE, true, false,
    std::bind(Callbacks::SetActiveTool, Tool::ROTATE)
  );
  _items.push_back(rotateItem);

  ToolbarButton* scaleItem = new ToolbarButton(
    this, Tool::SCALE, "Scale", "S", "scale tool",
    ICON_FA_UP_RIGHT_AND_DOWN_LEFT_FROM_CENTER , true, false,
    std::bind(Callbacks::SetActiveTool, Tool::SCALE)
  );
  _items.push_back(scaleItem);

  ToolbarButton* brushItem = new ToolbarButton(
    this, Tool::BRUSH, "Brush", "B", "brush tool",
    ICON_FA_PAINTBRUSH, true, false,
    std::bind(Callbacks::SetActiveTool, Tool::BRUSH)
  );
  _items.push_back(brushItem);

  _tools = {
    selectItem,
    translateItem,
    rotateItem,
    scaleItem,
    brushItem
  };

  ToolbarButton* playItem = new ToolbarButton(
    this, Tool::NONE, "Play", "play", "launch engine",
    ICON_FA_SHUFFLE, true, false,
    std::bind(Callbacks::ToggleExec)
  );
  _items.push_back(playItem);

  _parent->SetFixedSizeFunc(&ToolbarUIFixedSizeFunc);

}

ToolbarUI::~ToolbarUI() 
{
  for(auto& item: _items) delete item;
  _items.clear();
}

int
ToolbarUI::GetFixedSize()
{
  const ImGuiStyle &style = ImGui::GetStyle(); 
  if(_vertical)return UI::BUTTON_NORMAL_SIZE[0] + 2 * (style.WindowPadding.x + style.ItemSpacing.x);
  else return UI::BUTTON_NORMAL_SIZE[1] + 2 * (style.WindowPadding.y + style.ItemSpacing.y);
}


void
ToolbarUI::OnToolChangedNotice(const ToolChangedNotice& n)
{
  for(auto& button: _tools) 
    if(button->tool != n.GetTool())button->enabled = false;
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