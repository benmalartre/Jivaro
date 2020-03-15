#include "menu.h"
#include "../app/view.h"
#include "../app/window.h"

AMN_NAMESPACE_OPEN_SCOPE

// constructor
AmnMenuUI::AmnMenuUI(AmnView* parent):AmnUI(parent, "menu")
{
  parent->SetContent(this);
}

// destructor
AmnMenuUI::~AmnMenuUI()
{
  
}

// overrides
void AmnMenuUI::Event() 
{

}

void AmnMenuUI::Draw()
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

AMN_NAMESPACE_CLOSE_SCOPE