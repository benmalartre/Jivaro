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


MenuUI::Item::Item(MenuUI* ui, const std::string label, bool selected, bool enabled,
  Callback cb, const pxr::VtArray<pxr::VtValue>& args)
  : ui(ui)
  , parent(NULL)
  , label(label)
  , selected(selected)
  , enabled(enabled)
  , callback(cb)
  , args(args)
{
}

template<typename... Args>
constexpr std::size_t _Length(Args...)
{
  return sizeof...(Args);
}

template <typename A, typename ...Args>
struct _Unpack
{
  static void Process(pxr::VtArray<pxr::VtValue>& r, A&& a, Args && ...args)
  {
    r.push_back(pxr::VtValue(a));
    _Unpack<Args...>::Process(r, std::forward<Args>(args)...);
  }
};

template <typename A>
struct _Unpack<A>
{
  static void Process(pxr::VtArray<pxr::VtValue>& r, A&& a)
  {
    r.push_back(pxr::VtValue(a));
  }
};

template<typename... Args>
pxr::VtArray<pxr::VtValue> _UnpackVariadicParameters(Args... args)
{
  pxr::VtArray<pxr::VtValue> unpack;
  _Unpack<Args...>::Process(unpack, std::forward<Args>(args)...);
  return unpack;
}

MenuUI::Item& MenuUI::Item::Add(const std::string label,
  bool selected, bool enabled, Callback cb)
{
  items.push_back(MenuUI::Item(ui, label, selected, enabled, cb));
  return items.back();
}

template<typename... Args>
MenuUI::Item& MenuUI::Item::Add(const std::string label,
  bool selected, bool enabled, Callback cb, Args... args)
{
  items.push_back(MenuUI::Item(ui, label, selected, enabled, cb, _UnpackVariadicParameters(args...)));
  return items.back();
}

pxr::GfVec2i MenuUI::Item::ComputeSize()
{
  ImVec2 size(0, 0);
  ImGuiStyle& style = ImGui::GetStyle();
  for (const auto& item : items) {
    ImVec2 labelSize = ImGui::CalcTextSize(item.label.c_str());
    ImVec2 cur(labelSize[0] + 2 * style.FramePadding[0], labelSize[1]);
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
    if (ImGui::MenuItem(label.c_str()) && callback) {
      std::cout << "num arguments : " << args.size() << std::endl;
      callback(args[0].Get<int>(), args[1].Get<int>(), args[2].Get<int>());
      window->ForceRedraw();
      ui->_current = NULL;
    } 
  }
  return true;
}

static void OpenFileCallback(int i, int j, int k) {
  std::cout << i << "," << j << "," << k << std::endl;
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

static void NewFileCallback(int i, int j, int k)
{
  std::cout << i << "," << j << "," << k << std::endl;
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
  Window* childWindow = Application::CreateChildWindow(200, 200, 400, 400, mainWindow);
  app->AddWindow(childWindow);

  ViewportUI* viewport = new ViewportUI(childWindow->GetMainView());

  //DummyUI* dummy = new DummyUI(childWindow->GetMainView(), "Dummy");

  childWindow->CollectLeaves();
  mainWindow->SetGLContext();
}

static void SetLayoutCallback(Window* window, short layout)
{
  std::cout << "layout ? " << layout << std::endl;
  static size_t currentLayout = 0;
  std::cout << "set layout callback " << std::endl;
  GetApplication()->SetLayout(window, layout);

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


MenuUI::Item& MenuUI::Add(const std::string label, bool selected,
  bool enabled, Callback cb, const pxr::VtArray<pxr::VtValue>& args)
{
  _items.push_back(MenuUI::Item(this, label, selected, enabled, cb));
  return _items.back();
}

// constructor
MenuUI::MenuUI(View* parent) 
  : BaseUI(parent, UIType::MAINMENU)
  , _current(NULL)
{
  MenuUI::Item& fileMenu = Add("File", false, true, NULL);

  fileMenu.Add("Open", false, true, (Callback)OpenFileCallback, 1, 2, 3);
  fileMenu.Add("Save", false, true, (Callback)SaveFileCallback, 7, 6, 5);
  fileMenu.Add("New",  false, true, (Callback)NewFileCallback, 2, 2, 2);
 
  MenuUI::Item& testItem = Add("Test", false, true, NULL);
  testItem.Add("CreatePrim", false, true, (Callback)CreatePrimCallback);
  testItem.Add("Triangulate", false, true, (Callback)TriangulateCallback);
  /*
  MenuUI::Item& subItem = testItem.Add("SubMenu", false, true, NULL);
  subItem.Add("Sub0", false, true, NULL);
  subItem.Add("Sub1", false, true, NULL);
  subItem.Add("Sub2", false, true, NULL);

  MenuUI::Item& demoItem = Add("Demo", false, true);
  demoItem.Add("Open Demo", false, true, (Callback)OpenDemoCallback);
  demoItem.Add("Child Window", false, true, (Callback)OpenChildWindowCallback);

  static int layoutIdx = 0;
  MenuUI::Item& layoutItem = Add("Layout", false, true);
  layoutItem.Add("Standard", false, true, (Callback)SetLayoutCallback, GetWindow(), 0);
  layoutItem.Add("Raw", false, true, (Callback)SetLayoutCallback, GetWindow(), 1);
  */
  _parent->SetFlag(View::DISCARDMOUSEBUTTON);
}

// destructor
MenuUI::~MenuUI()
{
}
/*
template<typename Func, typename... Args>
MenuUI::Item& AddItem2(const std::string label, const std::string shortcut, bool selected,
  bool enabled, Func f, Args... args)
{
  _items.push_back(MenuUI::Item(this, label, shortcut, selected, enabled, func, a));
  return _items.back();
}

MenuUI::Item& MenuUI::AddItem(const std::string label, const std::string shortcut,
  bool selected, bool enabled, UIUtils::CALLBACK_FN func, const pxr::VtArray<pxr::VtValue> a)
{
  _items.push_back(MenuUI::Item(this, label, shortcut, selected, enabled, func, a));
  return _items.back();
}
*/
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