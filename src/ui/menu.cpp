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

PXR_NAMESPACE_OPEN_SCOPE


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

MenuItem::MenuItem(View* v, const std::string lbl, const std::string sht,
  bool sel, bool enb, MenuPressedFunc f, const pxr::VtArray<pxr::VtValue> a)
  : view(v), label(lbl), shortcut(sht), selected(sel), func(f), args(a)
{
}

MenuItem& MenuItem::AddItem(View* view, const std::string lbl, const std::string sht,
  bool sel, bool enb, MenuPressedFunc f, const pxr::VtArray<pxr::VtValue> a)
{
  items.push_back(MenuItem(view, lbl, sht, sel, enb, f, a));
  return items.back();
}

bool MenuItem::Draw()
{
  Window* window = view->GetWindow();
  if (items.size()) {
    ImGui::PushFont(window->GetBoldFont(1));
    
    if (ImGui::BeginMenu(label.c_str())) {
      for (auto& item : items) {
        item.Draw();
      }
      ImGui::EndMenu();
      ImGui::PopFont();
      return true;
    }
    ImGui::PopFont();
  }
  else {
    ImGui::PushFont(window->GetMediumFont(1));
    if (ImGui::MenuItem(label.c_str(), shortcut.c_str()) && func) {
      func(args);
      window->SetActiveTool(TOOL_SELECT);
      window->ForceRedraw();
      ImGui::PopFont();
      return true;
    }
    ImGui::PopFont();
  }
  return false;
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
      new CreatePrimCommand(GetApplication()->GetWorkStage(), name)));
}

static void FlattenGeometryCallback()
{
  Application* app = GetApplication();
  pxr::UsdStageRefPtr& stage = app->GetWorkStage();
  Selection* selection = app->GetSelection();
  for (size_t i = 0; i < selection->GetNumSelectedItems(); ++i) {
    Selection::Item& item = selection->GetItem(i);
    pxr::UsdPrim prim = stage->GetPrimAtPath(item.path);
    if (prim.IsValid() && prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::UsdGeomMesh mesh(prim);
      pxr::TfToken uvToken("st");
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
          /*
          cuts.clear();
          flattened.GetCutVerticesFromUVs(uvs, &cuts);
          */
          flattened.DisconnectEdges(cuts);
          flattened.Flatten(uvs, interpolation);

          mesh.GetPointsAttr().Set(pxr::VtValue(flattened.GetPositions()));
          mesh.GetFaceVertexCountsAttr().Set(pxr::VtValue(flattened.GetFaceCounts()));
          mesh.GetFaceVertexIndicesAttr().Set(pxr::VtValue(flattened.GetFaceConnects()));

          //pxr::VtArray<pxr::GfVec2d> uvs = uvPrimvar.Get< pxr::VtArray<pxr::GfVec2d>>()
        }
      }
    }
  }
}

// constructor
MenuUI::MenuUI(View* parent) :BaseUI(parent, "MainMenu")
{
  pxr::VtArray < pxr::VtValue > args;
  MenuItem& fileMenu = AddItem(parent, "File", "", false, true);
  fileMenu.AddItem(parent, "Open", "Ctrl+O", false, true, (MenuPressedFunc)&OpenFileCallback);
  fileMenu.AddItem(parent, "Save", "Ctrl+S", false, true, (MenuPressedFunc)&SaveFileCallback);
  fileMenu.AddItem(parent, "New", "Ctrl+N", false, true, (MenuPressedFunc)&NewFileCallback);
 
  MenuItem& testItem = AddItem(parent, "Test", "", false, true);
  testItem.AddItem(parent, "CreatePrim", "CTRL+P", false, true, (MenuPressedFunc)&CreatePrimCallback);

  MenuItem& demoItem = AddItem(parent, "Demo", "", false, true);
  demoItem.AddItem(parent, "Open Demo", "Shift+D", false, true, (MenuPressedFunc)&OpenDemoCallback);

}

// destructor
MenuUI::~MenuUI()
{
}

MenuItem& MenuUI::AddItem(View* view, const std::string label, const std::string shortcut,
  bool selected, bool enabled, MenuPressedFunc func, const pxr::VtArray<pxr::VtValue> a)
{
  _items.push_back(MenuItem(view, label, shortcut, selected, enabled, func, a));
  return _items.back();
}

// overrides
bool MenuUI::Draw()
{
  _parent->SetFlag(View::DISCARDMOUSEBUTTON);
  ImGui::PushStyleColor(ImGuiCol_Header, BACKGROUND_COLOR);
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ALTERNATE_COLOR);
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, SELECTED_COLOR);

  static bool open;
  const pxr::GfVec2f min(GetX(), GetY());
  const pxr::GfVec2f size(GetWidth(), GetHeight());

  ImGui::Begin("MenuBar", &open, _flags);
  ImGui::SetWindowPos(min);
  ImGui::SetWindowSize(size);
  
  ImDrawList* drawList = ImGui::GetWindowDrawList();

  drawList->AddRectFilled(
    min,
    min + size,
    ImColor(BACKGROUND_COLOR));

  
  ImGui::PushFont(GetWindow()->GetBoldFont(2));
  
  if (ImGui::BeginMainMenuBar())
  {
    _parent->SetFlag(View::INTERACTING | View::DISCARDMOUSEMOVE);
    for (auto& item : _items) {
      if (item.Draw()) {
        _parent->SetDirty();
        _parent->SetInteracting(true);
      }
    }
    ImGui::EndMainMenuBar();
  }
  
  ImGui::PopFont();
  ImGui::PopStyleColor(3);
  ImGui::End();

  return 
    ImGui::IsItemHovered() ||
    ImGui::IsAnyItemActive() ||
    ImGui::IsAnyItemFocused() ||
    ImGui::IsAnyMouseDown();
}

PXR_NAMESPACE_CLOSE_SCOPE