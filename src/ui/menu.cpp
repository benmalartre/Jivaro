#include <memory>
#include "../common.h"
#include "../ui/style.h"
#include "../ui/menu.h"
#include "../utils/strings.h"
#include "../command/command.h""
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
ImGuiWindowFlags_NoTitleBar |
ImGuiWindowFlags_NoMove |
ImGuiWindowFlags_NoResize |
ImGuiWindowFlags_NoBringToFrontOnFocus |
ImGuiWindowFlags_NoDecoration;

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
    ImGui::PushFont(window->GetBoldFont(0));
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
    ImGui::PushFont(window->GetMediumFont(0));
    if (ImGui::MenuItem(label.c_str(), shortcut.c_str()) && func) {
      func(args);

      window->SetActiveTool(TOOL_SELECT);
      view->ClearFlag(View::INTERACTING);
      window->ForceRedraw();
      ImGui::PopFont();
      return true;
    }
    ImGui::PopFont();
  }
  return false;
}

static void OpenFileCallback() {
  const char* folder = GetInstallationFolder().c_str();
  const char* filters[] = {
    ".usd",
    ".usda",
    ".usdc",
    ".usdz"
  };
  int numFilters = 4;

  std::string filename =
    GetApplication()->BrowseFile(200, 200, folder, filters, numFilters, "open usd file");

  GetApplication()->AddCommand(
    std::shared_ptr<OpenSceneCommand>(new OpenSceneCommand(filename)));
}

static void SaveFileCallback() {
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
    std::shared_ptr<CreatePrimCommand>(new CreatePrimCommand(GetApplication()->GetStage(), name)));
}

static void FlattenGeometryCallback()
{
  Application* app = GetApplication();
  pxr::UsdStageRefPtr& stage = app->GetStage();
  Selection* selection = app->GetSelection();
  std::cout << "NUM SELECTED ITEMS : " << selection->GetNumSelectedItems() << std::endl;
  for (size_t i = 0; i < selection->GetNumSelectedItems(); ++i) {
    Selection::Item& item = selection->GetItem(i);
    std::cout << "SELECTED PRIM PATH : " << item.path << std::endl;
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
  /*
  fileMenu.AddItem(parent, "Save", "Ctrl+S", false, true, (MenuPressedFunc)&SaveFileCallback);
  args.push_back(pxr::VtValue(7.0));

  MenuItem& testItem = AddItem(parent, "Test", "", false, true);
  testItem.AddItem(parent, "Flatten Geometry", "Shift+F", false, true, (MenuPressedFunc)&FlattenGeometryCallback);
  */
  MenuItem& createPrimItem = AddItem(parent, "Test", "", false, true);
  createPrimItem.AddItem(parent, "CreatePrim", "CTRL+P", false, true, (MenuPressedFunc)&CreatePrimCallback);

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
  ImGui::PushStyleColor(ImGuiCol_Header, BACKGROUND_COLOR);
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ALTERNATE_COLOR);
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, SELECTED_COLOR);
  Window* window = GetWindow();

  static bool open;
  ImGui::Begin("MenuBar", &open, _flags);

  ImGui::SetWindowPos(_parent->GetMin());
  ImGui::SetWindowSize(_parent->GetSize());
  ImDrawList* drawList = ImGui::GetBackgroundDrawList();

  drawList->AddRectFilled(
    _parent->GetMin(),
    _parent->GetMax(),
    ImGuiCol_WindowBg
  );

  if (ImGui::BeginMenuBar())
  {
    ImGui::PushFont(window->GetBoldFont(0));
    for (auto& item : _items) {
      if (item.Draw()) {
        _parent->SetDirty();
        _parent->SetInteracting(true);
      }
    }

    ImGui::PopFont();
    ImGui::EndMenuBar();
  }

  ImGui::PopStyleColor(3);
  ImGui::End();

  return
    ImGui::IsAnyItemActive() ||
    ImGui::IsAnyItemFocused() ||
    ImGui::IsAnyWindowHovered() ||
    ImGui::IsAnyMouseDown();
}

JVR_NAMESPACE_CLOSE_SCOPE