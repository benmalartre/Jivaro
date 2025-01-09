#include <memory>
#include <functional>
#include <tuple>

#include <pxr/base/vt/array.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/primvar.h>

#include "../common.h"
#include "../utils/strings.h"
#include "../ui/style.h"
#include "../ui/menu.h"
#include "../geometry/geometry.h"
#include "../command/manager.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/registry.h"
#include "../app/modal.h"
#include "../app/selection.h"
#include "../app/notice.h"
#include "../app/commands.h"
#include "../app/model.h"
#include "../pbd/menu.h"
#include "../exec/menu.h"


JVR_NAMESPACE_OPEN_SCOPE


// --------------------------------------------------------------
// Callbacks
// --------------------------------------------------------------

static void OpenFileCallback(MenuUI* menu) {
  std::string folder = GetInstallationFolder();
  const char* filters[] = {
    ".usd",
    ".usda",
    ".usdc",
    ".usdz"
  };
  int numFilters = 4;

  std::string filename =
    UI::BrowseFile(200, 200, folder.c_str(), filters, numFilters, "open usd file", false);
  ADD_COMMAND(OpenSceneCommand, filename);
}

static void SaveFileCallback(MenuUI* menu)
{
  SdfLayerHandle layer = menu->GetModel()->GetStage()->GetRootLayer();
  ADD_COMMAND(SaveLayerCommand, layer);
}

static void NewFileCallback(MenuUI* menu)
{

  std::string folder = GetInstallationFolder();
  const char* filters[] = {
    ".usd",
    ".usda",
    ".usdc",
    ".usdz"
  };
  int numFilters = 4;

  std::string filename =
    UI::BrowseFile(200, 200, folder.c_str(), filters, numFilters, "new usd file", true);
  ADD_COMMAND(NewSceneCommand, filename);
}


static void OpenChildWindowCallback(MenuUI* menu)
{
  WindowRegistry* registry = WindowRegistry::Get();
  Window* mainWindow = registry->GetWindow(0);
  Window* childWindow = registry->CreateChildWindow("Child Window", GfVec4i(200, 200, 400, 400), mainWindow);
  registry->AddWindow(childWindow);

  childWindow->SetDesiredLayout(1);

  mainWindow->SetGLContext();
}

static void SetLayoutCallback(Window* window, short layout)
{

  window->SetDesiredLayout(layout);
}

static void CreatePrimCallback(Model* model, const TfToken& type)
{
  Selection* selection = model->GetSelection();
  TfToken name(type.GetString() + "_" + RandomString(6));

  if (selection->GetNumSelectedItems()) {
    ADD_COMMAND(CreatePrimCommand, model->GetRootLayer(), selection->GetItem(0).path.AppendChild(name), type);
  }
  else {
    ADD_COMMAND(CreatePrimCommand, model->GetRootLayer(), SdfPath("/" + name.GetString()), type);
  }
}


ImGuiWindowFlags MenuUI::_flags =
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_MenuBar |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoCollapse |
  ImGuiWindowFlags_NoNav |
  ImGuiWindowFlags_NoScrollWithMouse |
  ImGuiWindowFlags_NoBringToFrontOnFocus |
  ImGuiWindowFlags_NoDecoration |
  ImGuiWindowFlags_NoBackground |
  ImGuiWindowFlags_NoScrollbar;


MenuUI::Item::Item(MenuUI* ui, Item* parent, const std::string& label, bool selected, bool enabled, CALLBACK_FN cb)
  : ui(ui)
  , parent(parent)
  , label(label)
  , selected(selected)
  , enabled(enabled)
  , callback(cb)
{
}

MenuUI::Item::~Item()
{
  for (auto& item : items)delete item;
}

MenuUI::Item* MenuUI::Item::Add(const std::string& label,
  bool selected, bool enabled, CALLBACK_FN cb)
{
  items.push_back(new MenuUI::Item(ui, this, label, selected, enabled, cb));
  return items.back();
}

void MenuUI::Item::Draw(bool* modified, size_t itemIdx)
{
  View* view = ui->GetView();
  Window* window = view->GetWindow();
  if(!parent)
    ImGui::PushFont(ui->GetFont(FONT_LARGE));
  else
    ImGui::PushFont(ui->GetFont(FONT_MEDIUM));
  if (items.size()) {
    if (ImGui::BeginMenu(label.c_str())) {
      ui->_opened.push_back(itemIdx);
      *modified = true;
      size_t subItemIdx = 0;
      for (auto& item : items) {
        item->Draw(modified, subItemIdx++);
      }
      ImGui::EndMenu();
    }
  } else {
    if (ImGui::MenuItem(label.c_str(), NULL, selected, enabled) && callback) {
      callback();
      WindowRegistry::Get()->SetAllWindowsDirty();
    }
  }
  ImGui::PopFont();
}

MenuUI::Item* MenuUI::Add(const std::string& label, bool selected, bool enabled, CALLBACK_FN cb)
{
  _items.push_back(new MenuUI::Item(this, NULL, label, selected, enabled, cb));
  return _items.back();
}

// constructor
MenuUI::MenuUI(View* parent)
  : BaseUI(parent, UIType::MAINMENU)
{
  MenuUI::Item* fileMenu = Add("File", false, true, NULL);

  fileMenu->Add("Open", false, true, std::bind(OpenFileCallback, this));
  fileMenu->Add("Save", false, true, std::bind(SaveFileCallback, this));
  fileMenu->Add("New", false, true, std::bind(NewFileCallback, this));

  MenuUI::Item* testItem = Add("Create", false, true, NULL);
  testItem->Add("Create Plane", false, true, std::bind(CreatePrimCallback, _model, TfToken("Plane")));
  testItem->Add("Create Cube", false, true, std::bind(CreatePrimCallback, _model, TfToken("Cube")));
  testItem->Add("Create Sphere", false, true, std::bind(CreatePrimCallback, _model, TfToken("Sphere")));
  testItem->Add("Create Cylinder", false, true, std::bind(CreatePrimCallback, _model, TfToken("Cylinder")));
  testItem->Add("Create Capsule", false, true, std::bind(CreatePrimCallback, _model, TfToken("Capsule")));
  testItem->Add("Create Cone", false, true, std::bind(CreatePrimCallback, _model, TfToken("Cone")));

  AddPbdMenu(this);
  AddExecMenu(this);

  MenuUI::Item* demoItem = Add("Demo", false, true);
  demoItem->Add("Child Window", false, true, std::bind(OpenChildWindowCallback, this));

  static int layoutIdx = 0;
  MenuUI::Item* layoutItem = Add("Layout", false, true);
  layoutItem->Add("Base", false, true, std::bind(SetLayoutCallback, GetWindow(), 0));
  layoutItem->Add("Raw", false, true, std::bind(SetLayoutCallback, GetWindow(), 1));
  layoutItem->Add("Standard", false, true, std::bind(SetLayoutCallback, GetWindow(), 2));
  layoutItem->Add("Random", false, true, std::bind(SetLayoutCallback, GetWindow(), 3));

  _parent->SetFixedSizeFunc(&MenuUIFixedSizeFunc);
}

// destructor
MenuUI::~MenuUI()
{
  for(auto & item: _items)delete item;
}

int
MenuUIFixedSizeFunc(BaseUI* ui)
{
  return ui->GetView()->GetWindow()->GetMenuBarHeight();
}

GfVec2f
MenuUI::_ComputeSize(const MenuUI::Item* item)
{
  GfVec2f size(0, 0);
  ImGuiStyle& style = ImGui::GetStyle();
  for (const auto& subItem : item->items) {
    GfVec2f labelSize = ImGui::CalcTextSize(subItem->label.c_str());
    GfVec2f cur(labelSize + GfVec2f(style.ItemSpacing.x , ImGui::GetTextLineHeightWithSpacing()));
    if (cur[0] > size[0])size[0] = cur[0];
    size[1] += cur[1];
  }
  return GfVec2f(size[0], size[1]);
}

GfVec2f
MenuUI::_ComputePos(const MenuUI::Item* item)
{
  ImGuiStyle& style = ImGui::GetStyle();
  View* view = item->ui->GetView();
  GfVec2f pos(0.f);
  if (item->parent) {
    pos[0] += _ComputeSize(item->parent)[0];
    for (auto& subItem : item->parent->items) {
      if (item->label == subItem->label) break;
      GfVec2f cur = ImGui::CalcTextSize(subItem->label.c_str());
      pos[1] += cur[1] + style.ItemSpacing[1];
    }
  }
  else {
    pos += GfVec2f(view->GetX() + style.WindowPadding[0], view->GetY());
    for (auto& subItem : item->ui->_items) {
      if (item->label == subItem->label.c_str()) break;
      GfVec2f cur = ImGui::CalcTextSize(subItem->label.c_str()) + GfVec2f(0, ImGui::GetTextLineHeightWithSpacing());
      pos[0] += cur[0] + style.ItemSpacing[0];
    }
  }
  
  return pos;
}

void 
MenuUI::MouseButton(int button, int action, int mods)
{
  if (action == GLFW_PRESS) _parent->SetInteracting(true);
}

void 
MenuUI::DirtyViewsBehind()
{
  size_t numOpened = _opened.size();

  ImDrawList* drawList = ImGui::GetForegroundDrawList();

  if (_opened.size()) {
    MenuUI::Item* current = _items[_opened[0]];
    GfVec2f pos = _ComputePos(current);
    GfVec2f size = _ComputeSize(current);
    drawList->AddRect(pos, pos + size, ImColor(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1, 1.f), 2.f);
    GetWindow()->DirtyViewsUnderBox(pos, size);

    for (size_t openedIdx = 1; openedIdx < numOpened; ++openedIdx) {
      MenuUI::Item* child = current->items[_opened[openedIdx]];
      pos += _ComputePos(child);
      size = _ComputeSize(child);
      drawList->AddRect(pos, pos + size, ImColor(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1, 1.f), 2.f);
      GetWindow()->DirtyViewsUnderBox(pos, size);
      current = child;
    }
    
  }
  else {
    const ImGuiStyle& style = ImGui::GetStyle();
    GfVec2f pos = GfVec2f(0, 0);
    GfVec2f size = GfVec2f(GetWidth(), ImGui::GetTextLineHeightWithSpacing() + 2 * style.FramePadding.y);

    drawList->AddRect(pos, pos + size, ImColor(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1, 1.f), 2.f);
    _parent->GetWindow()->DirtyViewsUnderBox(pos, size);
  }
  
  
}

// overrides
bool 
MenuUI::Draw()
{
  const ImGuiStyle& style = ImGui::GetStyle();
  ImGui::PushStyleColor(ImGuiCol_Header, style.Colors[ImGuiCol_WindowBg]);
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, style.Colors[ImGuiCol_ChildBg]);
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, style.Colors[ImGuiCol_ButtonActive]);

  bool dirty = false;
  _opened.clear();
  if (ImGui::BeginMainMenuBar())
  {
    size_t itemIdx = 0;
    for (auto& item : _items)
      item->Draw(&dirty, itemIdx++);
    ImGui::EndMainMenuBar();
  }
  if (dirty || ImGui::IsAnyItemHovered()) { DirtyViewsBehind(); }
  else { SetInteracting(false); };

  ImGui::PopStyleColor(3);

  return dirty || ImGui::IsAnyItemHovered();

}




JVR_NAMESPACE_CLOSE_SCOPE