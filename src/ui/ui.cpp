#include "ui.h"
#include "../app/window.h"
#include "../app/view.h"


AMN_NAMESPACE_OPEN_SCOPE

// constructor
BaseUI::BaseUI(View* parent, const std::string& name, bool docked)
  : _parent(parent) 
  , _name(name)
  , _docked(docked)
  , _initialized(false)
{
  if(_parent)
  {
    _parent->SetContent(this);
    _parent->SetFlag(View::LEAF);
  }
  pxr::TfWeakPtr<BaseUI> me(this);
  //pxr::TfNotice::Register(me, &BaseUI::OnAllNotices);
  pxr::TfNotice::Register(me, &BaseUI::OnNewSceneNotice);
};

void BaseUI::OnNewSceneNotice(const Notice::NewScene& n)
{
  std::cout << "PROCESS NEW SCENE !!!" << std::endl;
  _initialized = false;
}

void BaseUI::OnAllNotices(const pxr::TfNotice& n)
{
  std::cout << "PROCESS ALL NOTICES !!!" << std::endl;
}

// mouse positon relative to the view
void BaseUI::GetRelativeMousePosition(const float inX, const float inY, float& outX, float& outY)
{
  pxr::GfVec2f parentPosition = _parent->GetMin();
  float parentX = parentPosition[0];
  float parentY = parentPosition[1];
  outX = inX - parentX;
  outY = inY - parentY;
}

// parent window
Window* BaseUI::GetWindow(){return _parent->GetWindow();};

// parent window height
int BaseUI::GetWindowHeight(){return _parent->GetWindow()->GetHeight();};
//void BaseUI::SetWindowContext(){_parent->GetWindow()->SetContext();};

// ui dimensions
ImVec2 BaseUI::GetPosition()
{
  if (_docked)return ImVec2(_parent->GetMin()[0] - 1, _parent->GetMin()[1] - 1);
  else return ImVec2(ImGui::GetWindowPos()[0] - 1, ImGui::GetWindowPos()[1] - 1);
}

int BaseUI::GetX()
{
  if(_docked)return _parent->GetMin()[0] - 1;
  else return ImGui::GetWindowPos().x - 1;
};

int BaseUI::GetY()
{ 
  if(_docked)return _parent->GetMin()[1] - 1;
    //return GetWindowHeight() - (_parent->GetMin()[1] + _parent->GetHeight());
  else return ImGui::GetWindowPos().y - 1;
};

int BaseUI::GetWidth()
{ 
  if(_docked)return _parent->GetWidth() + 2;
  else return ImGui::GetWindowSize().x + 2;
};

int BaseUI::GetHeight()
{
  if(_docked)return _parent->GetHeight() + 2;
  else return ImGui::GetWindowSize().y + 2;
};

Application* BaseUI::GetApplication()
{
  return _parent->GetWindow()->GetApplication();
}



AMN_NAMESPACE_CLOSE_SCOPE
