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
  return pxr::GfVec2i(size[0], size[1]);
}

pxr::GfVec2i MenuUI::Item::ComputePos()
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

void MenuUI::Item::Draw()
{
  View* view = ui->GetView();
  Window* window = view->GetWindow();
  if (items.size()) {
    //ImGui::PushFont(window->GetBoldFont(1));
    
    if (ImGui::BeginMenu(label.c_str())) {
      ui->_current = this;
      for (auto& item : items) {
        item.Draw();
      }
      ImGui::EndMenu();
    } 
    //ImGui::PopFont();
    
  }
  else {
    //ImGui::PushFont(window->GetMediumFont(1));
    if (ImGui::MenuItem(label.c_str(), shortcut.c_str()) && func) {
      func(args);
      window->ForceRedraw();
      ui->_current = NULL;
    } 
    //ImGui::PopFont();
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
      new CreatePrimCommand(GetApplication()->GetCurrentLayer(), name)));
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

void MenuUI::DirtyViewsUnderBox()
{
  if (_current) {
    _pos = _current->ComputePos();
    _size = _current->ComputeSize();
    _parent->GetWindow()->DirtyViewsUnderBox(_pos, _size);
  } else {
    _parent->GetWindow()->DirtyViewsUnderBox(pxr::GfVec2i(0, 0), pxr::GfVec2i(GetWidth(), 256));
  }
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

  const pxr::GfVec2f min(GetX(), GetY());
  const pxr::GfVec2f size(GetWidth(), GetHeight());

  ImGui::Begin("MenuBar", NULL, _flags);
  ImGui::SetWindowPos(min);
  ImGui::SetWindowSize(size);
  
  ImDrawList* drawList = ImGui::GetWindowDrawList();

  drawList->AddRectFilled(min, min + size, ImColor(style.Colors[ImGuiCol_WindowBg]));

  if (ImGui::BeginMainMenuBar())
  {
    for (auto& item : _items) {
      item.Draw();
    }
    ImGui::EndMainMenuBar();
  }

  bool dirty = _current || ImGui::IsPopupOpen("##MainMenuBar", ImGuiPopupFlags_AnyPopup) || ImGui::IsItemClicked();
  if (dirty) {DirtyViewsUnderBox();}

  ImDrawList* foregroundList = ImGui::GetForegroundDrawList();
  foregroundList->AddRect(ImVec2(_pos), ImVec2(_pos+_size), ImColor(255,128,128,255));
  
  ImGui::PopStyleColor(3);
  ImGui::End();

  return ImGui::IsAnyItemHovered();
  /*
  //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 8));
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("New")) {
        //DrawModalDialog<CreateUsdFileModalDialog>(*this);
      }
      if (ImGui::MenuItem("Open")) {
        //DrawModalDialog<OpenUsdFileModalDialog>(*this);
      }
      if (ImGui::BeginMenu("Open Recent (as stage)")) {
        
        for (const auto& recentFile : _settings._recentFiles) {
          if (ImGui::MenuItem(recentFile.c_str())) {
            ExecuteAfterDraw<EditorOpenStage>(recentFile);
          }
        }
        
        ImGui::EndMenu();
      }
      ImGui::Separator();
      const bool hasLayer = false;// GetCurrentLayer() != SdfLayerRefPtr();
      if (ImGui::MenuItem("Save layer", "CTRL+S", false, hasLayer)) {
        //GetCurrentLayer()->Save(true);
      }
      if (ImGui::MenuItem("Save current layer as", "CTRL+F", false, hasLayer)) {
        //ExecuteAfterDraw<EditorSaveLayerAs>(GetCurrentLayer());
      }

      ImGui::Separator();
      if (ImGui::MenuItem("Quit")) {
        //RequestShutdown();
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
      if (ImGui::MenuItem("Undo", "CTRL+Z")) {
        //ExecuteAfterDraw<UndoCommand>();
      }
      if (ImGui::MenuItem("Redo", "CTRL+R")) {
        //ExecuteAfterDraw<RedoCommand>();
      }
      if (ImGui::MenuItem("Clear Undo/Redo")) {
        //ExecuteAfterDraw<ClearUndoRedoCommand>();
      }
      if (ImGui::MenuItem("Clear History")) {
        //_layerHistory.clear();
        //_layerHistoryPointer = 0;
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Cut", "CTRL+X", false, false)) {
      }
      if (ImGui::MenuItem("Copy", "CTRL+C", false, false)) {
      }
      if (ImGui::MenuItem("Paste", "CTRL+V", false, false)) {
      }
      ImGui::EndMenu();
    }


    ImGui::EndMainMenuBar();
  }
  return false;
  */
}

JVR_NAMESPACE_CLOSE_SCOPE