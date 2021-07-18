#include "../common.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/application.h"
#include "../ui/style.h"
#include "../app/modal.h"
#include "menu.h"

AMN_NAMESPACE_OPEN_SCOPE

extern Application* AMN_APPLICATION;

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
      
      window->SetActiveTool(AMN_TOOL_SELECT);
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
  Application* app = AMN_APPLICATION;
  const char* folder = GetInstallationFolder().c_str();
  const char* filters[] = {
    ".usd",
    ".usda",
    ".usdc",
    ".usdz"
  };
  int numFilters = 4;

  std::string filename =
    app->BrowseFile(folder, filters, numFilters, "open usd file");
  app->OpenScene(filename);
}

static void SaveFileCallback() {
  std::cout << "ON SAVE CALLBACK !!!" << std::endl;
}
static void OpenDemoCallback()
{
  ModalDemo demo("Demo");
  demo.Loop();
  demo.Term();
}

// constructor
MenuUI::MenuUI(View* parent):BaseUI(parent, "MainMenu")
{
  pxr::VtArray < pxr::VtValue > args;
  MenuItem& fileMenu = AddItem(parent, "File", "", false, true);
  fileMenu.AddItem(parent, "Open", "Ctrl+O", false, true, (MenuPressedFunc)&OpenFileCallback);
  fileMenu.AddItem(parent, "Save", "Ctrl+S", false, true, (MenuPressedFunc)&SaveFileCallback);
  args.push_back(pxr::VtValue(7.0));
  
  MenuItem& demoItem = AddItem(parent, "Demo", "", false, true);
  demoItem.AddItem(parent, "OpenDemo", "Shift+D", false, true, (MenuPressedFunc)&OpenDemoCallback);
}

// destructor
MenuUI::~MenuUI()
{
}


MenuItem& MenuUI::AddItem(View* view, const std::string label, const std::string shortcut, 
  bool selected, bool enabled, MenuPressedFunc f, const pxr::VtArray<pxr::VtValue> a)
{
  _items.push_back(MenuItem(view, label, shortcut, selected, enabled, f, a));
  return _items.back();
}

// overrides
bool MenuUI::Draw()
{  
  ImGui::PushStyleColor(ImGuiCol_Header, AMN_BACKGROUND_COLOR);
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, AMN_ALTERNATE_COLOR);
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, AMN_SELECTED_COLOR);
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
    /*
    if (ImGui::BeginMenu("Edit"))
    {
      _parent->SetDirty();
      _parent->SetInteracting(true);
      //ImGui::PushFont(window->GetMediumFont());
      if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
        std::cout << "UNDO !!!" << std::endl;
      }
      if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {
        std::cout << "REDO !!!" << std::endl;
      }  // Disabled item
      ImGui::Separator();
      if (ImGui::MenuItem("Cut", "Ctrl+X")) {
        std::cout << "CUT !!!" << std::endl;
      }
      if (ImGui::MenuItem("Copy", "Ctrl+C")) {
        std::cout << "COPY !!!" << std::endl;
      }
      if (ImGui::MenuItem("Paste", "Ctrl+V")) {
        std::cout << "PASTE !!!" << std::endl;
      }
      ImGui::EndMenu();
    }
    */
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

AMN_NAMESPACE_CLOSE_SCOPE