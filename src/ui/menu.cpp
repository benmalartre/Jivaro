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


MenuUI::Item*
MenuUI::Item::Add(const std::string label, bool selected, bool enabled, CALLBACK_FN cb)
{
  items.push_back({ this->ui, this, label, selected, enabled, cb });
  return &items.back();
}

pxr::GfVec2i MenuUI::ComputeSize(const MenuUI::Item& item)
{
  ImVec2 size(0, 0);
  ImGuiStyle& style = ImGui::GetStyle();
  for (const auto& subItem : item.items) {
    ImVec2 labelSize = ImGui::CalcTextSize(subItem.label.c_str());
    ImVec2 cur(labelSize[0] + 2 * style.FramePadding[0], labelSize[1]);
    if (cur[0] > size[0])size[0] = cur[0];
    size[1] += cur[1] + style.ItemSpacing[1] + 2 * style.ItemInnerSpacing[1];
  }
  return pxr::GfVec2i(size[0] * 2, size[1] * 2);
}

pxr::GfVec2i MenuUI::ComputePos(const MenuUI::Item& item)
{
  ImGuiStyle& style = ImGui::GetStyle();
  View* view = GetView();
  pxr::GfVec2i pos(view->GetX() + style.WindowPadding[0], view->GetY());
  for (auto& subItem : item.items) {
    if (item.label == subItem.label) break;
    ImVec2 cur = ImGui::CalcTextSize(subItem.label.c_str());
    pos[0] += cur[0] + style.ItemSpacing[0];
  }
  return pos;
}

bool MenuUI::_Draw(const MenuUI::Item& item)
{
  if (item.items.size()) {    
    if (ImGui::BeginMenu(item.label.c_str())) {
      for (auto& subItem : item.items) {
        if(_Draw(subItem))return true;
      }
      ImGui::EndMenu();
    } 
  } else {
    if (ImGui::MenuItem(item.label.c_str())&& item.callback) {
      item.callback();
      return true;
    } 
  }
  return false;
}

MenuUI::Item*
MenuUI::Add(const std::string label, bool selected, bool enabled, CALLBACK_FN cb)
{
  _items.push_back({ this, NULL, label, selected, enabled, cb });
  return &_items.back();
}


// constructor
MenuUI::MenuUI(View* parent)
  : BaseUI(parent, UIType::MAINMENU)
  /*, _current(NULL)*/
{
  Window* window = GetWindow();
  MenuUI::Item* fileMenu = Add("File", false, true, NULL);

  fileMenu->Add("Open", false, true, std::bind(OpenFileCallback));
  fileMenu->Add("Save", false, true, std::bind(SaveFileCallback));
  fileMenu->Add("New", false, true, std::bind(NewFileCallback));
  /*
  MenuUI::Item& testMenu = Add("Test", false, true, NULL);
  testMenu.Add("CreatePrim", false, true, std::bind(CreatePrimCallback));
  testMenu.Add("Triangulate", false, true, std::bind(TriangulateCallback));

  MenuUI::Item& subMenu = testMenu.Add("SubMenu", false, true, NULL);
  subMenu.Add("Sub0", false, true, NULL);
  subMenu.Add("Sub1", false, true, NULL);
  subMenu.Add("Sub2", false, true, NULL);

  MenuUI::Item& demoMenu = Add("Demo", false, true);
  demoMenu.Add("Open Demo", false, true, std::bind(OpenDemoCallback));
  demoMenu.Add("Child Window", false, true, std::bind(OpenChildWindowCallback));*/

  MenuUI::Item* layoutMenu = Add("Layout", false, true);
  layoutMenu->Add("Base", false, true, std::bind(SetLayoutCallback, window, 0));
  layoutMenu->Add("Raw", false, true, std::bind(SetLayoutCallback, window, 1));
  layoutMenu->Add("Standard", false, true, std::bind(SetLayoutCallback, window, 2));
  layoutMenu->Add("Random", false, true, std::bind(SetLayoutCallback, window, 3));

  _parent->SetFlag(View::DISCARDMOUSEBUTTON);
}

// destructor
MenuUI::~MenuUI()
{
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
  /*
  if (_current) {
    _pos = _current->ComputePos();
    _size = _current->ComputeSize();
  }
  else {
    */
    _pos = pxr::GfVec2i(0, 0);
    _size = pxr::GfVec2i(GetWidth(), 128);
  //}
  _parent->GetWindow()->DirtyViewsUnderBox(_pos, _size);
  _parent->SetDirty();
}

// overrides
bool MenuUI::Draw()
{
  const ImGuiStyle& style = ImGui::GetStyle();
  //if (!_parent->IsActive())_current = NULL;
  ImGui::PushStyleColor(ImGuiCol_Header, style.Colors[ImGuiCol_WindowBg]);
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, style.Colors[ImGuiCol_ChildBg]);
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, style.Colors[ImGuiCol_ButtonActive]);

  bool active = false;
  if (ImGui::BeginMainMenuBar())
  {
    for (auto& item : _items) {
      if (_Draw(item))active = true;
    }
    ImGui::EndMainMenuBar();
  }

  //bool dirty = (_current || active);
  if (active) { DirtyViewsUnderBox(); }


  ImGui::PopStyleColor(3);

  return _parent->IsInteracting() || ImGui::IsAnyItemHovered();

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
  std::string filename =
    app->BrowseFile(200, 200, folder.c_str(), filters, numFilters, "open usd file");
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
  Window* childWindow = Application::CreateChildWindow("ChildWindow", pxr::GfVec4i(200, 200, 400, 400), mainWindow);
  app->AddWindow(childWindow);

  childWindow->SetLayout(1);

  mainWindow->SetGLContext();
}

static void CreatePrimCallback()
{
  std::string name = RandomString(32);

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

static void SetLayoutCallback(Window* window, size_t layout)
{
  window->SetLayout(layout);
}

JVR_NAMESPACE_CLOSE_SCOPE