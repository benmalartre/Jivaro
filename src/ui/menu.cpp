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


pxr::GfVec2f 
MenuUI::_ComputeSize(const MenuUI::Item* item)
{
  ImVec2 size(0, 0);
  ImGuiStyle& style = ImGui::GetStyle();
  for (const auto& subItem : item->items) {
    ImVec2 labelSize = ImGui::CalcTextSize(subItem.label.c_str());
    ImVec2 cur(labelSize[0] + 2 * style.FramePadding[0], labelSize[1]);
    if (cur[0] > size[0])size[0] = cur[0];
    size[1] += cur[1] + style.ItemSpacing[1] + 2 * style.ItemInnerSpacing[1];
  }
  return pxr::GfVec2f(size[0], size[1]);
}

pxr::GfVec2f
MenuUI::_ComputePos(const MenuUI::Item* item, size_t subIndex)
{
  ImGuiStyle& style = ImGui::GetStyle();
  View* view = GetView();
  pxr::GfVec2f pos(0,0);
  if (item->parent) {
    for (size_t idx = 0; idx < subIndex; ++idx) {
      const pxr::GfVec2f cur = ImGui::CalcTextSize(_items[idx].label.c_str());
      if (cur[0] > pos[0])pos[0] = cur[0];
      pos[1] += cur[1] + style.ItemSpacing[0];
    }
  }
  else {
    for (size_t idx = 0; idx < subIndex; ++idx) {
      const pxr::GfVec2f cur = ImGui::CalcTextSize(item->items[idx].label.c_str());
      pos[0] += cur[0] + style.ItemSpacing[0];
    }
  }
  return pos;
}

bool
MenuUI::_Draw(const MenuUI::Item& item, size_t relativeIndex)
{
  if (item.items.size()) {    
    if (ImGui::BeginMenu(item.label.c_str())) {
      _opened.push_back(relativeIndex);
      size_t subItemIndex = 0;
      for (auto& subItem : item.items) {
        if(_Draw(subItem, subItemIndex++))return true;
      }
      ImGui::EndMenu();
    } 
  } else {
    if (ImGui::MenuItem(item.label.c_str())&& item.callback) {
      item.callback();
      GetWindow()->ForceRedraw();
      _opened.clear();
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

MenuUI::Item*
MenuUI::Item::Add(const std::string label, bool selected, bool enabled, CALLBACK_FN cb)
{
  items.push_back({ this->ui, this, label, selected, enabled, cb });
  return &items.back();
}


// constructor
MenuUI::MenuUI(View* parent)
  : BaseUI(parent, UIType::MAINMENU)
{
  Window* window = GetWindow();
  MenuUI::Item* fileMenu = Add("File", false, true, NULL);

  fileMenu->Add("Open", false, true, std::bind(OpenFileCallback));
  fileMenu->Add("Save", false, true, std::bind(SaveFileCallback));
  fileMenu->Add("New", false, true, std::bind(NewFileCallback));
  
  MenuUI::Item* testMenu = Add("Test", false, true, NULL);
  testMenu->Add("CreatePrim", false, true, std::bind(CreatePrimCallback));
  testMenu->Add("Triangulate", false, true, std::bind(TriangulateCallback));

  MenuUI::Item* subMenu = testMenu->Add("SubMenu", false, true, NULL);
  subMenu->Add("Sub0", false, true, NULL);
  subMenu->Add("Sub1", false, true, NULL);
  MenuUI::Item* subSubMenu = subMenu->Add("Sub2", false, true, NULL);

  subSubMenu->Add("SubSub0", false, true, NULL);
  subSubMenu->Add("SubSub1", false, true, NULL);
  subSubMenu->Add("SubSub2", false, true, NULL);

  MenuUI::Item* demoMenu = Add("Demo", false, true);
  demoMenu->Add("Open Demo", false, true, std::bind(OpenDemoCallback));
  demoMenu->Add("Child Window", false, true, std::bind(OpenChildWindowCallback));

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

void 
MenuUI::MouseButton(int button, int action, int mods)
{
  _parent->SetDirty();
  if (action == GLFW_PRESS)
    _parent->SetInteracting(true);
  else if (action == GLFW_RELEASE)
    _parent->SetInteracting(false);
}

void 
MenuUI::DirtyViewsBehind()
{
  if (!_opened.size())return;

  ImGuiStyle& style = ImGui::GetStyle();
  View* view = GetView();
  pxr::GfVec2f pos(view->GetX() + style.WindowPadding[0], view->GetY() + style.WindowPadding[1]);
  pxr::GfVec2f size(GetWidth(), 128);

  MenuUI::Item* current = &_items[_opened[0]];
  pos += _ComputePos(NULL, _opened[0]);
  size = _ComputeSize(current);
  current->pos = pos;
  current->size = size;
  _parent->GetWindow()->DirtyViewsUnderBox(pos, size);
  for (size_t subIdx = 1; subIdx < _opened.size(); ++subIdx) {
    const pxr::GfVec2f subPos = _ComputePos(current, _opened[subIdx]);
    const pxr::GfVec2f subSize = _ComputeSize(&current->items[subIdx]);
    pos += subPos + size;
    current->items[subIdx].pos = pos;
    current->items[subIdx].size = subSize;
    _parent->GetWindow()->DirtyViewsUnderBox(pos, subSize);
    size = subSize;
  }

  _parent->SetDirty();
  
}

// overrides
bool 
MenuUI::Draw()
{
  _opened.clear();
  const ImGuiStyle& style = ImGui::GetStyle();
  if (!_parent->IsActive() && _opened.size())_opened.clear();
  ImGui::PushStyleColor(ImGuiCol_Header, style.Colors[ImGuiCol_WindowBg]);
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, style.Colors[ImGuiCol_ChildBg]);
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, style.Colors[ImGuiCol_ButtonActive]);

  bool active = false;
  if (ImGui::BeginMainMenuBar())
  {
    size_t itemIndex = 0;
    for (auto& item : _items) {
      if (_Draw(item, itemIndex++))active = true;
    }
    ImGui::EndMainMenuBar();
  }

  if (_opened.size() || active) {
    DirtyViewsBehind();
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    
    MenuUI::Item* current = NULL;
    for (auto& opened : _opened) {
      if (current) {
        current = &current->items[opened];
      }
      else {
        current = &_items[opened];
      }
      drawList->AddRect(current->pos, current->pos + current->size, ImColor({ RANDOM_0_1,RANDOM_0_1, RANDOM_0_1, 1.f }), 4.f, 0, 4.f);
    }

      

    pxr::GfVec2f pos = GetPosition() + pxr::GfVec2f(100, 200);
    drawList->AddText(pos, ImColor({ 0,255,0,255 }), "opened : ");
    pos += pxr::GfVec2f(64, 0);

    for (auto& opened : _opened) {
      pos += pxr::GfVec2f(32, 0);
      drawList->AddText(pos, ImColor({ 0,255,0,255 }), (std::to_string(opened) + ",").c_str());
    }
  }

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

  childWindow->SetDesiredLayout(1);

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
  window->SetDesiredLayout(layout);
}

JVR_NAMESPACE_CLOSE_SCOPE