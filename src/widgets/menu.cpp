#include "../common.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/application.h"
#include "menu.h"

AMN_NAMESPACE_OPEN_SCOPE

extern Application* AMN_APPLICATION;

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
      
      window->SetActiveTool(AMN_TOOL_NONE);
      std::cout << "SET WINDOW TOOL TO NONE !!!" << std::endl;
      std::cout << "IS MENU ACTIVE : " << (window->GetActiveTool() == AMN_TOOL_MENU) << std::endl;
      view->ClearFlag(View::INTERACTING);
      window->ForceRedraw();
      ImGui::PopFont();
      return true;
    }
    ImGui::PopFont();
  }
  return false;
}

static size_t fileIdx = 0;
static void OpenFileCommand() {
  switch (fileIdx % 3) {
    case 0:
      AMN_APPLICATION->OpenScene("E:/Projects/RnD/USD_BUILD/assets/maneki_anim.usd");
      break;
    case 1:
      AMN_APPLICATION->OpenScene("E:/Projects/RnD/USD_BUILD/assets/Bottles.usda");
      break;
    case 2:
      AMN_APPLICATION->OpenScene("E:/Projects/RnD/USD_BUILD/assets/Kitchen_set/Kitchen_set.usd");
      break;
  }
  fileIdx++;
}

// constructor
MenuUI::MenuUI(View* parent):BaseUI(parent, "MainMenu")
{
  pxr::VtArray < pxr::VtValue > args;
  MenuItem& fileMenu = AddItem(parent, "File", "", false, true);
  fileMenu.AddItem(parent, "Open", "Ctrl+O", false, true, (MenuPressedFunc)&OpenFileCommand);
  fileMenu.AddItem(parent, "Save", "Ctrl+S", false, true, (MenuPressedFunc)&OpenFileCommand);
  args.push_back(pxr::VtValue(7.0));

  MenuItem& testItem = AddItem(parent, "Test", "", false, true);
  testItem.AddItem(parent, "Child1", "", true, true, (MenuPressedFunc)&TestMenuCallback, args);
  testItem.AddItem(parent, "Child2", "", true, false, (MenuPressedFunc)&TestMenuCallback, args);
  testItem.AddItem(parent, "Child3", "", false, false, (MenuPressedFunc)&TestMenuCallback, args);
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

static void ShowExampleMenuFile()
{

    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
    if (ImGui::BeginMenu("Open Recent"))
    {
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More.."))
        {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor");
            if (ImGui::BeginMenu("Recurse.."))
            {
                ShowExampleMenuFile();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}
    ImGui::Separator();
    if (ImGui::BeginMenu("Options"))
    {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), true);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        static bool b = true;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::Checkbox("Check", &b);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Colors"))
    {
        float sz = ImGui::GetTextLineHeight();
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x+sz, p.y+sz), ImGui::GetColorU32((ImGuiCol)i));
            ImGui::Dummy(ImVec2(sz, sz));
            ImGui::SameLine();
            ImGui::MenuItem(name);
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Disabled", false)) // Disabled
    {
        IM_ASSERT(0);
    }
    if (ImGui::MenuItem("Checked", NULL, true)) {}
    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}

// overrides
bool MenuUI::Draw()
{  
  Window* window = GetWindow();
  if (ImGui::BeginMainMenuBar())
  {
    ImGui::PushFont(window->GetBoldFont(0));
    /*
    if(ImGui::BeginMenu("File"))
    {
      _parent->SetDirty();
      _parent->SetInteracting(true);
      //ImGui::PushFont(window->GetMediumFont());
      ShowExampleMenuFile();
      ImGui::EndMenu();
      //ImGui::PopFont();
    }*/
    if (ImGui::BeginMenu("Edit"))
    {
      _parent->SetDirty();
      _parent->SetInteracting(true);
      //ImGui::PushFont(window->GetMediumFont());
      if (ImGui::MenuItem("Undo", "CTRL+Z")) 
      {
        std::cout << "UNDO !!!" << std::endl;
      }
      if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {
        std::cout << "REDO !!!" << std::endl;
      }  // Disabled item
      ImGui::Separator();
      if (ImGui::MenuItem("Cut", "CTRL+X")) {}
      if (ImGui::MenuItem("Copy", "CTRL+C")) {}
      if (ImGui::MenuItem("Paste", "CTRL+V")) {}
      ImGui::EndMenu();
      //ImGui::PopFont();
    }
    if (ImGui::BeginMenu("Demo"))
    {
      _parent->SetDirty();
      _parent->SetInteracting(true);
      ImGui::PushFont(window->GetMediumFont(0));
      if (ImGui::MenuItem("Open", "CTRL+D")) 
      {
        _showDemoWindow = true;
        ImGui::ShowDemoWindow();
      }
      if (ImGui::MenuItem("Close", "SHIFT+D")) 
      {
        _showDemoWindow = false;
      }
      ImGui::PopFont();
      ImGui::EndMenu();
    }

    for (auto& item : _items) {
      if (item.Draw()) {
        _parent->SetDirty();
        _parent->SetInteracting(true);
      }
    }

    ImGui::PopFont();
    ImGui::EndMainMenuBar();
  }

  return
    ImGui::IsAnyItemActive() ||
    ImGui::IsAnyItemFocused() ||
    ImGui::IsAnyWindowHovered();

} 

AMN_NAMESPACE_CLOSE_SCOPE