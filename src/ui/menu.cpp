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

MenuItem::MenuItem(MenuUI* ui, const std::string lbl, const std::string sht,
  bool sel, bool enb, MenuPressedFunc f, const pxr::VtArray<pxr::VtValue>& a)
  : ui(ui), label(lbl), shortcut(sht), selected(sel), func(f), args(a)
{
}

MenuItem& MenuItem::AddItem(MenuUI* ui, const std::string lbl, const std::string sht,
  bool sel, bool enb, MenuPressedFunc f, const pxr::VtArray<pxr::VtValue>& a)
{
  items.push_back(MenuItem(ui, lbl, sht, sel, enb, f, a));
  return items.back();
}

pxr::GfVec2i MenuItem::GetSize()
{
  ImVec2 size(0, 0);
  ImGuiStyle& style = ImGui::GetStyle();
  std::cout << "MENU ITEM GET SIZE ..." << std::endl;
  for (const auto& item : items) {
    std::cout << "ITEM : " << item.label << std::endl;
    ImVec2 labelSize = ImGui::CalcTextSize(item.label.c_str());
    ImVec2 shortcutSize = ImGui::CalcTextSize(item.shortcut.c_str());
    ImVec2 cur(labelSize[0] + shortcutSize[0] + 2 * style.FramePadding[0], labelSize[1]);
    if (cur[0] > size[0])size[0] = cur[0];
    size[1] += cur[1] + style.ItemSpacing[1] + 2 * style.ItemInnerSpacing[1];
  }
  std::cout << "MENU ITEM SIZE : " << size[0] << "," << size[1] << std::endl;
  return pxr::GfVec2i(size[0], size[1]);
}

pxr::GfVec2i MenuItem::GetPos()
{
  ImGuiStyle& style = ImGui::GetStyle();
  View* view = ui->GetView();
  pxr::GfVec2i pos(view->GetX() + style.WindowPadding[0], view->GetY() + view->GetHeight());
  for (auto& item : ui->_items) {
    if (label == item.label) break;
    ImVec2 cur = ImGui::CalcTextSize(item.label.c_str());
    pos[0] += cur[0] + style.ItemSpacing[0];
  }
  return pos;
}

void MenuItem::Draw()
{
  View* view = ui->GetView();
  Window* window = view->GetWindow();
  if (items.size()) {
    ImGui::PushFont(window->GetBoldFont(1));
    
    if (ImGui::BeginMenu(label.c_str())) {
      ui->_current = this;
      for (auto& item : items) {
        item.Draw();
      }
      ImGui::EndMenu();
    } 
    ImGui::PopFont();
    
  }
  else {
    ImGui::PushFont(window->GetMediumFont(1));
    if (ImGui::MenuItem(label.c_str(), shortcut.c_str()) && func) {
      func(args);
      window->ForceRedraw();
      ui->_current = NULL;
    } 
    ImGui::PopFont();
  }
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
  : BaseUI(parent, "MainMenu")
  , _current(NULL)
{
  pxr::VtArray < pxr::VtValue > args;
  MenuItem& fileMenu = AddItem("File", "", false, true);
  fileMenu.AddItem(this, "Open", "Ctrl+O", false, true, (MenuPressedFunc)&OpenFileCallback);
  fileMenu.AddItem(this, "Save", "Ctrl+S", false, true, (MenuPressedFunc)&SaveFileCallback);
  fileMenu.AddItem(this, "New", "Ctrl+N", false, true, (MenuPressedFunc)&NewFileCallback);
 
  MenuItem& testItem = AddItem("Test", "", false, true);
  testItem.AddItem(this, "CreatePrim", "CTRL+P", false, true, (MenuPressedFunc)&CreatePrimCallback);
  MenuItem& subItem = testItem.AddItem(this, "SubMenu", "", false, true);
  subItem.AddItem(this, "Sub0", "", false, true);
  subItem.AddItem(this, "Sub1", "", false, true);
  subItem.AddItem(this, "Sub2", "", false, true);

  MenuItem& demoItem = AddItem("Demo", "", false, true);
  demoItem.AddItem(this, "Open Demo", "Shift+D", false, true, (MenuPressedFunc)&OpenDemoCallback);

  _parent->SetFlag(View::DISCARDMOUSEBUTTON);
}

// destructor
MenuUI::~MenuUI()
{
}

MenuItem& MenuUI::AddItem(const std::string label, const std::string shortcut,
  bool selected, bool enabled, MenuPressedFunc func, const pxr::VtArray<pxr::VtValue> a)
{
  _items.push_back(MenuItem(this, label, shortcut, selected, enabled, func, a));
  return _items.back();
}

void MenuUI::DirtyViewsUnderBox()
{
  _parent->GetWindow()->DirtyViewsUnderBox(pxr::GfVec2i(0, 0), pxr::GfVec2i(256, 256));
  _parent->SetDirty();
  _pos = _current->GetPos();
  _size = _current->GetSize();
}

// overrides
bool MenuUI::Draw()
{
  if (!_parent->IsActive())_current = NULL;
  ImGui::PushStyleColor(ImGuiCol_Header, BACKGROUND_COLOR);
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ALTERNATE_COLOR);
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, SELECTED_COLOR);

  const pxr::GfVec2f min(GetX(), GetY());
  const pxr::GfVec2f size(GetWidth(), GetHeight());

  ImGui::Begin("MenuBar", NULL, _flags);
  ImGui::SetWindowPos(min);
  ImGui::SetWindowSize(size);
  
  ImDrawList* drawList = ImGui::GetWindowDrawList();

  drawList->AddRectFilled(min, min + size, ImColor(BACKGROUND_COLOR));

  ImGui::PushFont(GetWindow()->GetBoldFont(2));
  if (ImGui::BeginMainMenuBar())
  {
    for (auto& item : _items) {
      item.Draw();
    }
    ImGui::EndMainMenuBar();
  }

  bool dirty = _current /*|| ImGui::IsPopupOpen("##MainMenuBar", ImGuiPopupFlags_AnyPopup) || ImGui::IsItemClicked()*/;
  if (dirty) {
    DirtyViewsUnderBox();
  }

  ImDrawList* foregroundList = ImGui::GetForegroundDrawList();
  foregroundList->AddRect(ImVec2(_pos), ImVec2(_pos+_size), ImColor(255,128,128,255));
  
  ImGui::PopFont();
  ImGui::PopStyleColor(3);
  ImGui::End();

  return true;
}

JVR_NAMESPACE_CLOSE_SCOPE