#include "../default.h"
#include "menu.h"
#include "../app/view.h"
#include "../app/window.h"

AMN_NAMESPACE_OPEN_SCOPE

// constructor
MenuUI::MenuUI(View* parent):BaseUI(parent, "MainMenu"){}

// destructor
MenuUI::~MenuUI(){}

// Note that shortcuts are currently provided for display only (future version will add flags to BeginMenu to process shortcuts)
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
void MenuUI::Draw()
{  
  if (ImGui::BeginMainMenuBar())
  {
    if (ImGui::BeginMenu("File"))
    {
      ShowExampleMenuFile();
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit"))
    {
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
    }
    if (ImGui::BeginMenu("Demo"))
    {
      if (ImGui::MenuItem("Open", "CTRL+D")) 
      {
        _showDemoWindow = true;
        ImGui::ShowDemoWindow();
      }
      if (ImGui::MenuItem("Close", "SHIFT+D")) 
      {
        _showDemoWindow = false;
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
  if(_showDemoWindow) ImGui::ShowDemoWindow();
} 

AMN_NAMESPACE_CLOSE_SCOPE