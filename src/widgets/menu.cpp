#include "menu.h"
#include "../view.h"

namespace AMN {

  // constructor
  MenuUI::MenuUI(View* parent):UI(parent, "menu")
  {
    parent->SetContent(this);
  }

  // destructor
  MenuUI::~MenuUI()
  {
    
  }

  // overrides
  void MenuUI::Event() 
  {

  }

  void MenuUI::Draw()
  {  
    int flags = 0;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoMove;
    
    bool open;
    ImGui::Begin("TopMenu", &open, flags);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Close")) open = false;
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::SetWindowSize(_parent->GetMax() - _parent->GetMin());
    ImGui::SetWindowPos(_parent->GetMin());
    ImGui::End();

  } 
} // namespace AMN