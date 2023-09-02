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
#include "../app/view.h"
#include "../app/window.h"
#include "../app/application.h"
#include "../app/modal.h"
#include "../app/selection.h"
#include "../app/notice.h"
#include "../app/commands.h"


JVR_NAMESPACE_OPEN_SCOPE

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
  }
  else {
    if (ImGui::MenuItem(label.c_str(), NULL, selected, enabled) && callback) {
      callback();
      window->ForceRedraw();
    }
  }
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

  fileMenu->Add("Open", false, true, std::bind(OpenFileCallback));
  fileMenu->Add("Save", false, true, std::bind(SaveFileCallback));
  fileMenu->Add("New", false, true, std::bind(NewFileCallback));

  MenuUI::Item* testItem = Add("Test", false, true, NULL);
  testItem->Add("CreatePrim", false, true, std::bind(CreatePrimCallback));
  testItem->Add("Triangulate", false, true, std::bind(TriangulateCallback));

  MenuUI::Item* subItem = testItem->Add("SubMenu", false, true, NULL);
  subItem->Add("Sub0", false, true, NULL);
  subItem->Add("Sub1", false, true, NULL);
  MenuUI::Item* subSubItem = subItem->Add("Sub2", false, true, NULL);

  subSubItem->Add("SubSub0", false, true, NULL);
  subSubItem->Add("SubSub1", true, true, NULL);
  subSubItem->Add("SubSub2", false, false, NULL);
  subSubItem->Add("SubSub3", true, false, NULL);
  subSubItem->Add("SubSub4", false, true, NULL);
  subSubItem->Add("SubSub5", false, true, NULL);

  MenuUI::Item* demoItem = Add("Demo", false, true);
  demoItem->Add("Open Demo", false, true, std::bind(OpenDemoCallback));
  demoItem->Add("Child Window", false, true, std::bind(OpenChildWindowCallback));

  static int layoutIdx = 0;
  MenuUI::Item* layoutItem = Add("Layout", false, true);
  layoutItem->Add("Base", false, true, std::bind(SetLayoutCallback, GetWindow(), 0));
  layoutItem->Add("Raw", false, true, std::bind(SetLayoutCallback, GetWindow(), 1));
  layoutItem->Add("Standard", false, true, std::bind(SetLayoutCallback, GetWindow(), 2));
  layoutItem->Add("Random", false, true, std::bind(SetLayoutCallback, GetWindow(), 3));

  _parent->SetFlag(View::DISCARDMOUSEBUTTON);
}

// destructor
MenuUI::~MenuUI()
{
  for(auto & item: _items)delete item;
}

pxr::GfVec2f
MenuUI::_ComputeSize(const MenuUI::Item* item)
{
  pxr::GfVec2f size(0, 0);
  ImGuiStyle& style = ImGui::GetStyle();
  for (const auto& subItem : item->items) {
    pxr::GfVec2f labelSize = ImGui::CalcTextSize(subItem->label.c_str());
    pxr::GfVec2f cur(labelSize + pxr::GfVec2f(style.ItemSpacing.x , ImGui::GetTextLineHeightWithSpacing()));
    if (cur[0] > size[0])size[0] = cur[0];
    size[1] += cur[1];
  }
  return pxr::GfVec2f(size[0], size[1]);
}

pxr::GfVec2f
MenuUI::_ComputePos(const MenuUI::Item* item)
{
  ImGuiStyle& style = ImGui::GetStyle();
  View* view = item->ui->GetView();
  pxr::GfVec2f pos(0.f);
  if (item->parent) {
    pos[0] += _ComputeSize(item->parent)[0];
    for (auto& subItem : item->parent->items) {
      if (item->label == subItem->label) break;
      pxr::GfVec2f cur = ImGui::CalcTextSize(subItem->label.c_str());
      pos[1] += cur[1] + style.ItemSpacing[1];
    }
  }
  else {
    pos += pxr::GfVec2f(view->GetX() + style.WindowPadding[0], view->GetY());
    for (auto& subItem : item->ui->_items) {
      if (item->label == subItem->label.c_str()) break;
      pxr::GfVec2f cur = ImGui::CalcTextSize(subItem->label.c_str()) + pxr::GfVec2f(0, ImGui::GetTextLineHeightWithSpacing());
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
    pxr::GfVec2f pos = _ComputePos(current);
    pxr::GfVec2f size = _ComputeSize(current);
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
    pxr::GfVec2f pos = pxr::GfVec2f(0, 0);
    pxr::GfVec2f size = pxr::GfVec2f(GetWidth(), ImGui::GetTextLineHeightWithSpacing() + 2 * style.FramePadding.y);

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
    for (auto& item : _items)item->Draw(&dirty, itemIdx++);
    ImGui::EndMainMenuBar();
  }
  if (dirty || ImGui::IsAnyItemHovered()) { DirtyViewsBehind(); }
  else { SetInteracting(false); };

  ImGui::PopStyleColor(3);
  return dirty || ImGui::IsAnyItemHovered();

}

// --------------------------------------------------------------
// Callbacks (maybe should live in another file)
// --------------------------------------------------------------
static void OpenFileCallback() {
  std::string folder = GetInstallationFolder();
  const char* filters[] = {
    ".usd",
    ".usda",
    ".usdc",
    ".usdz"
  };
  int numFilters = 4;

  Application* app = GetApplication();
  std::string filename =
    app->BrowseFile(200, 200, folder.c_str(), filters, numFilters, "open usd file");
  app->OpenScene(filename);
}

static void SaveFileCallback()
{
  GetApplication()->SaveScene();
}

static void NewFileCallback()
{
  std::string folder = GetInstallationFolder();
  const char* filters[] = {
    ".usd",
    ".usda",
    ".usdc",
    ".usdz"
  };
  int numFilters = 4;

  Application* app = GetApplication();
  std::string filename = "hello_world";
  ADD_COMMAND(NewSceneCommand, filename);
}

static void OpenDemoCallback()
{
  ModalDemo demo(0, 0, "Demo");
  demo.Loop();
  demo.Term();
}

static void OpenChildWindowCallback()
{
  Application* app = GetApplication();
  Window* mainWindow = app->GetMainWindow();
  Window* childWindow = Application::CreateChildWindow("Child Window", pxr::GfVec4i(200, 200, 400, 400), mainWindow);
  app->AddWindow(childWindow);

  childWindow->SetDesiredLayout(1);

  mainWindow->SetGLContext();
}

static void SetLayoutCallback(Window* window, short layout)
{
  window->SetDesiredLayout(layout);
}

static void CreatePrimCallback()
{
  pxr::SdfPath name(RandomString(32));

  ADD_COMMAND(CreatePrimCommand, GetApplication()->GetCurrentLayer(), name);
}

static void TriangulateCallback()
{
  Application* app = GetApplication();
  const pxr::UsdStageRefPtr& stage = app->GetWorkStage();
  Selection* selection = app->GetSelection();
  for (size_t i = 0; i < selection->GetNumSelectedItems(); ++i) {
    Selection::Item& item = selection->GetItem(i);
    pxr::UsdPrim prim = stage->GetPrimAtPath(item.path);
    /*
    if (prim.IsValid() && prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::UsdGeomMesh mesh(prim);
      Mesh triangulated(mesh);
      triangulated.UpdateTopologyFromHalfEdges();
      mesh.GetPointsAttr().Set(pxr::VtValue(triangulated.GetPositions()));
      mesh.GetFaceVertexCountsAttr().Set(pxr::VtValue(triangulated.GetFaceCounts()));
      mesh.GetFaceVertexIndicesAttr().Set(pxr::VtValue(triangulated.GetFaceConnects()));
    }
    */
    std::cout << "triangulate : " << prim.GetPath() << std::endl;
  }
}

static void FlattenGeometryCallback()
{
  Application* app = GetApplication();
  const pxr::UsdStageRefPtr& stage = app->GetWorkStage();
  Selection* selection = app->GetSelection();
  for (size_t i = 0; i < selection->GetNumSelectedItems(); ++i) {
    Selection::Item& item = selection->GetItem(i);
    pxr::UsdPrim prim = stage->GetPrimAtPath(item.path);
    if (prim.IsValid() && prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::UsdGeomMesh mesh(prim);
      pxr::TfToken uvToken("st");
      /*
      if (mesh.HasPrimvar(uvToken)) {
        std::cout << "WE FOUND UVS :)" << std::endl;
        pxr::UsdGeomPrimvar uvPrimvar = mesh.GetPrimvar(uvToken);
        pxr::TfToken interpolation = uvPrimvar.GetInterpolation();
        if (interpolation == pxr::UsdGeomTokens->vertex) {
          std::cout << "UV INTERPOLATION VERTEX !" << std::endl;
          pxr::VtArray<pxr::GfVec2d> uvs;
          uvPrimvar.Get(&uvs);
          std::cout << "UVS COUNT : " << uvs.size() << std::endl;
        }
        else if (interpolation == pxr::UsdGeomTokens->faceVarying) {
          std::cout << "UV INTERPOLATION FACE VARYING !" << std::endl;
          pxr::VtArray<pxr::GfVec2d> uvs;
          uvPrimvar.Get(&uvs);

          std::cout << "UVS COUNT : " << uvs.size() << std::endl;
          // pxr::UsdGeomMesh usdFlattened = pxr::UsdGeomMesh::Define(stage, pxr::SdfPath(item.path.GetString() + "_flattened"));
          Mesh flattened(mesh);
          std::cout << flattened.GetNumPoints() << std::endl;
          pxr::VtArray<int> cuts;
          flattened.GetCutEdgesFromUVs(uvs, &cuts);

          //cuts.clear();
          //flattened.GetCutVerticesFromUVs(uvs, &cuts);

          flattened.DisconnectEdges(cuts);
          flattened.Flatten(uvs, interpolation);

          mesh.GetPointsAttr().Set(pxr::VtValue(flattened.GetPositions()));
          mesh.GetFaceVertexCountsAttr().Set(pxr::VtValue(flattened.GetFaceCounts()));
          mesh.GetFaceVertexIndicesAttr().Set(pxr::VtValue(flattened.GetFaceConnects()));

          //pxr::VtArray<pxr::GfVec2d> uvs = uvPrimvar.Get< pxr::VtArray<pxr::GfVec2d>>()
        }
      }
      */
    }
  }
}

JVR_NAMESPACE_CLOSE_SCOPE