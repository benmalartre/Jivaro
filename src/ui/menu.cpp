#include <memory>
#include "../common.h"
#include "../ui/style.h"
#include "../ui/menu.h"
#include "../utils/strings.h"
#include "../command/command.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/application.h"
#include "../app/modal.h"
#include "../app/selection.h"
#include "../app/notice.h"


#include <pxr/base/vt/array.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/primvar.h>

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

MenuUI::Item::Item(MenuUI* ui, const std::string lbl, const std::string sht,
  bool sel, bool enb, MenuUI::PressedFunc f, const pxr::VtArray<pxr::VtValue>& a)
  : ui(ui), label(lbl), shortcut(sht), selected(sel), func(f), args(a), parent(NULL)
{
}

MenuUI::Item& MenuUI::Item::AddItem(MenuUI* ui, const std::string lbl, const std::string sht,
  bool sel, bool enb, MenuUI::PressedFunc f, const pxr::VtArray<pxr::VtValue>& a)
{
  items.push_back(MenuUI::Item(ui, lbl, sht, sel, enb, f, a));
  return items.back();
}

MenuUI::Item::Item(MenuUI::Item* parent, const std::string lbl, const std::string sht,
  bool sel, bool enb, MenuUI::PressedFunc f, const pxr::VtArray<pxr::VtValue>& a)
  : ui(parent->ui), label(lbl), shortcut(sht), selected(sel), func(f), args(a), parent(parent)
{
}

MenuUI::Item& MenuUI::Item::AddItem(MenuUI::Item* parent, const std::string lbl, const std::string sht,
  bool sel, bool enb, MenuUI::PressedFunc f, const pxr::VtArray<pxr::VtValue>& a)
{
  items.push_back(MenuUI::Item(parent, lbl, sht, sel, enb, f, a));
  return items.back();
}

pxr::GfVec2i MenuUI::Item::ComputeSize()
{
  ImVec2 size(0, 0);
  ImGuiStyle& style = ImGui::GetStyle();
  for (const auto& item : items) {
    ImVec2 labelSize = ImGui::CalcTextSize(item.label.c_str());
    ImVec2 shortcutSize = ImGui::CalcTextSize(item.shortcut.c_str());
    ImVec2 cur(labelSize[0] + shortcutSize[0] + 2 * style.FramePadding[0], labelSize[1]);
    if (cur[0] > size[0])size[0] = cur[0];
    size[1] += cur[1] + style.ItemSpacing[1] + 2 * style.ItemInnerSpacing[1];
  }
  return pxr::GfVec2i(size[0] * 2, size[1] * 2);
}

pxr::GfVec2i MenuUI::Item::ComputePos()
{
  ImGuiStyle& style = ImGui::GetStyle();
  View* view = ui->GetView();
  pxr::GfVec2i pos(view->GetX() + style.WindowPadding[0], view->GetY());
  for (auto& item : ui->_items) {
    if (label == item.label) break;
    ImVec2 cur = ImGui::CalcTextSize(item.label.c_str());
    pos[0] += cur[0] + style.ItemSpacing[0];
  }
  return pos;
}

bool MenuUI::Item::Draw()
{
  View* view = ui->GetView();
  Window* window = view->GetWindow();
  if (items.size()) {    
    if (ImGui::BeginMenu(label.c_str())) {
      ui->_current = this;
      for (auto& item : items) {
        item.Draw();
      }
      ImGui::EndMenu();
    } 
  }
  else {
    if (ImGui::MenuItem(label.c_str(), shortcut.c_str()) && func) {
      func(args);
      window->ForceRedraw();
      ui->_current = NULL;
    } 
  }
  return true;
}

static void OpenFileCallback() {
  std::string folder = GetInstallationFolder();
  const char* filters[] = {
    ".usd",
    ".usda",
    ".usdc",
    ".usdz"
  };
  int numFilters = 4;

  std::string filename =
    GetApplication()->BrowseFile(200, 200, folder.c_str(), filters, numFilters, "open usd file");

  GetApplication()->AddCommand(
    std::shared_ptr<OpenSceneCommand>(new OpenSceneCommand(filename)));
}

static void SaveFileCallback() 
{
  GetApplication()->SaveScene();
}

static void NewFileCallback() 
{
  GetApplication()->AddCommand(
    std::shared_ptr<NewSceneCommand>(new NewSceneCommand()));
}

static void OpenDemoCallback()
{
  ModalDemo demo(0, 0, "Demo");
  demo.Loop();
  demo.Term();
}

static void CreatePrimCallback()
{
  std::string name = RandomString(32);

  GetApplication()->AddCommand(
    std::shared_ptr<CreatePrimCommand>(
      new CreatePrimCommand(GetApplication()->GetCurrentLayer(), name)));
}

static void TriangulateCallback()
{
  Application* app = GetApplication();
  const pxr::UsdStageRefPtr& stage = app->GetWorkStage();
  Selection* selection = app->GetSelection();
  for (size_t i = 0; i < selection->GetNumSelectedItems(); ++i) {
    Selection::Item& item = selection->GetItem(i);
    pxr::UsdPrim prim = stage->GetPrimAtPath(item.path);
    if (prim.IsValid() && prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::UsdGeomMesh mesh(prim);
      Mesh triangulated(mesh);
      triangulated.UpdateTopologyFromHalfEdges();
      mesh.GetPointsAttr().Set(pxr::VtValue(triangulated.GetPositions()));
      mesh.GetFaceVertexCountsAttr().Set(pxr::VtValue(triangulated.GetFaceCounts()));
      mesh.GetFaceVertexIndicesAttr().Set(pxr::VtValue(triangulated.GetFaceConnects()));
    }
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

// constructor
MenuUI::MenuUI(View* parent) 
  : BaseUI(parent, UIType::MAINMENU)
  , _current(NULL)
{
  pxr::VtArray < pxr::VtValue > args;
  MenuUI::Item& fileMenu = AddItem("File", "", false, true);
  fileMenu.AddItem(this, "Open", "Ctrl+O", false, true, (MenuUI::PressedFunc)&OpenFileCallback);
  fileMenu.AddItem(this, "Save", "Ctrl+S", false, true, (MenuUI::PressedFunc)&SaveFileCallback);
  fileMenu.AddItem(this, "New", "Ctrl+N", false, true, (MenuUI::PressedFunc)&NewFileCallback);
 
  MenuUI::Item& testItem = AddItem("Test", "", false, true);
  testItem.AddItem(this, "CreatePrim", "CTRL+P", false, true, (MenuUI::PressedFunc)&CreatePrimCallback);
  testItem.AddItem(this, "Triangulate", "CTRL+P", false, true, (MenuUI::PressedFunc)&TriangulateCallback);
  MenuUI::Item& subItem = testItem.AddItem(this, "SubMenu", "", false, true);
  subItem.AddItem(&subItem, "Sub0", "", false, true);
  subItem.AddItem(&subItem, "Sub1", "", false, true);
  subItem.AddItem(&subItem, "Sub2", "", false, true);

  MenuUI::Item& demoItem = AddItem("Demo", "", false, true);
  demoItem.AddItem(this, "Open Demo", "Shift+D", false, true, (MenuUI::PressedFunc)&OpenDemoCallback);

  _parent->SetFlag(View::DISCARDMOUSEBUTTON);
}

// destructor
MenuUI::~MenuUI()
{
}

MenuUI::Item& MenuUI::AddItem(const std::string label, const std::string shortcut,
  bool selected, bool enabled, MenuUI::PressedFunc func, const pxr::VtArray<pxr::VtValue> a)
{
  _items.push_back(MenuUI::Item(this, label, shortcut, selected, enabled, func, a));
  return _items.back();
}

void MenuUI::MouseButton(int button, int action, int mods)
{
  _parent->SetDirty();
  if (action == GLFW_PRESS)
    _parent->SetInteracting(true);
  else if (action == GLFW_RELEASE)
    _parent->SetInteracting(false);
}

void MenuUI::DirtyViewsUnderBox()
{
  if (_current) {
    _pos = _current->ComputePos();
    _size = _current->ComputeSize();
  }
  else {
    _pos = pxr::GfVec2i(0, 0);
    _size = pxr::GfVec2i(GetWidth(), 128);
  }
  _parent->GetWindow()->DirtyViewsUnderBox(_pos, _size);
  _parent->SetDirty();
  ImDrawList* foregroundList = ImGui::GetForegroundDrawList();
  foregroundList->AddRect(ImVec2(_pos), ImVec2(_pos + _size), ImColor(255, 128, 128, 255));
}

// overrides
bool MenuUI::Draw()
{
  const ImGuiStyle& style = ImGui::GetStyle();
  if (!_parent->IsActive())_current = NULL;
  ImGui::PushStyleColor(ImGuiCol_Header, style.Colors[ImGuiCol_WindowBg]);
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, style.Colors[ImGuiCol_ChildBg]);
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, style.Colors[ImGuiCol_ButtonActive]);

  bool active = false;
  if (ImGui::BeginMainMenuBar())
  {
    for (auto& item : _items) {
      if(item.Draw())active = true;
    }
    ImGui::EndMainMenuBar();
  }

  bool dirty = _current || active;
  if (dirty) {DirtyViewsUnderBox();}
  
  
  ImGui::PopStyleColor(3);

  return _parent->IsInteracting() || ImGui::IsAnyItemHovered();

}

JVR_NAMESPACE_CLOSE_SCOPE